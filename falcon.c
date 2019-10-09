
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include "falcon.h"

#define CHILD_STACK_SIZE (1024 * 1024)
#define BUFFER_SIZE (1024)


struct thread
{
  pid_t pid;
  char* stack_bot;
  char* stack_top;
  int dead;
  struct thread* next;
};


struct thread_queue
{
  int num_threads;
  struct thread* head;
};

struct thread_queue tq = { 0, NULL };


int main(const int argc, const char** argv)
{
  struct options opts;
  memset(&opts, 0, sizeof(opts));

  get_options(argc, argv, &opts);

  start_server(&opts);

  return 0;
}


void manage_threads()
{
  // continuously check thread queue for closed connections
  while (1)
  {
    struct thread* h = tq->head;
    while (h != NULL)
    {
      if (h->dead) cleanup(h->pid);
      h = h->next;
    }
    sleep(10);
  }
}


void get_options(const int argc, const char** argv, struct options* opts)
{
  char* usage = "Usage: ./FalconServer [-a address] [-p port] [-c clients]\n";

  for (int i = 1; i < argc; i++)
  {
    if (strcmp("-p", argv[i]) == 0)
    {
      unsigned int port = 0;
      if (i + 1 < argc)
        port = strtol(argv[i+1], NULL, 10);
      else
      {
        fprintf(stderr, "No value supplied for option -p.\n%s", usage);
        exit(-1);
      }
      if (port < 1 || port > 65535)
      {
        fprintf(stderr, "Invalid port number: %s\n%s", argv[i+1], usage);
        exit(-1);
      }
      opts->server_port = port;
      i++;
    }
    else if (strcmp("-c", argv[i]) == 0)
    {
      int clients = 0;
      if (i + 1 < argc)
        clients = strtol(argv[i+1], NULL, 10);
      else
      {
        fprintf(stderr, "No value supplied for option -c.\n%s", usage);
        exit(-1);
      }
      if (clients < 1)
      {
        fprintf(stderr, "Invalid number of clients: %s\n%s", argv[i+1], usage);
        exit(-1);
      }
      opts->max_clients = clients;
      i++;
    }
    else if (strcmp("-a", argv[i]) == 0)
    {
      struct sockaddr_in sa;
      if (i + 1 < argc)
      {
        if (inet_pton(AF_INET, argv[i+1], &(sa.sin_addr)) == 0) 
        {
          fprintf(stderr, "Invalid server IP address: %s\n", argv[i+1]);
	  exit(-1);
        }
      }
      else
      {
        fprintf(stderr, "No value supplied for option -a.\n%s", usage);
	exit(-1);
      }
      opts->server_ip = sa.sin_addr.s_addr;
      i++;
    }
    else
    {
      fprintf(stderr, "Unknown option: %s\n%s", argv[i], usage);
      exit(-1);
    }
  }

  // defaults
  if (opts->server_port == 0) opts->server_port = htons(80);
  if (opts->max_clients == 0) opts->max_clients = 10;
  // default ip is already set to 0.0.0.0
}// end valid_args


void start_server(const struct options* opts)
{
  int server_fd, client_fd, bytes_read;
  struct sockaddr_in server_addr, client_addr;
  int addr_len = sizeof(struct sockaddr_in);
  char* greeting = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 13\n\nSup Mauricio!";
  char in_buffer[1000] = {0};
  pid_t thread_manager_pid = 0;

  memset(&server_addr, 0, addr_len);
  memset(&client_addr, 0, addr_len);
  
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = opts->server_port;
  server_addr.sin_addr.s_addr = opts->server_ip;

  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
  {
    perror("Failed to create socket");
    exit(-1);
  }

  if (bind(server_fd, (struct sockaddr*) &server_addr, addr_len) < 0)
  {
    perror("Failed to bind socket to address");
    exit(-1);
  }

  if (listen(server_fd, opts->max_clients) < 0)
  {
    perror("Failed to listen for connections");
    exit(-1);
  }

  printf("Listening on port %d...\n", ntohs(opts->server_port));
  while (1)
  {
    if ((client_fd = accept(server_fd, (struct sockaddr*) &client_addr, &addr_len)) < 0)
      perror("Failed to accept incoming connection");
    else if (new_connection(client_fd, (struct sockaddr*) &client_addr) == -1)
      exit(-1);
  }

  return 0;
}


int new_connection(int fd, struct sockaddr* address)
{
  struct thread* t = calloc(1, sizeof(struct thread));
  t->stack_bot = malloc(CHILD_STACK_SIZE);
  t->stack_top = stack + CHILD_STACK_SIZE;

  /* add to queue prior to starting to
   * ensure thread can only die after it
   * is on the queue
   */
  add_to_queue(t);

  if ((t->pid = clone(&serve_connection, stack_top, )) == -1)
  {
    perror("Failed to create new thread");
    remove_from_queue(t->pid);
  }
  
  return t->pid;
}


int serve_connection(int fd, struct sockaddr* address, struct thread* t)
{
  int err = 0;
  int in_bytes = 0;
  char in_buffer[BUFFER_SIZE] = {0};

  while(1)
  {
    if ((in_bytes = recv(fd, in_buffer, BUFFER_SIZE, 0)) < 0)
      break;

    if (in_bytes > 0)
    {
      err = evaluate(fd, address, in_bytes, in_buffer);
      memset(in_buffer, 0, BUFFER_SIZE);
    }
  }

  close(client_fd);

  char ip_str[16] = {0};
  
  if (inet_ntop(address->sa_family, &address->sin_addr, ip_str, 16) == NULL)
  {
    perror("inet_ntop");
    printf("Connection closed\n");
  }
  else printf("Connection with %s closed.\n", );

  free(address);
  t->dead = 1;
  return err;
}



