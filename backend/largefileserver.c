
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>


#define BUFFERLEN 1024


struct options
{
  uint16_t server_port;
  uint32_t server_ip;
  int max_clients;
  int max_queue;
};


static void get_options(const int argc, const char** argv, struct options* opts);


static void start_server(const struct options* opts);


int main(const int argc, const char** argv)
{
  struct options opts;
  memset(&opts, 0, sizeof(opts));

  get_options(argc, argv, &opts);

  start_server(&opts);

  return 0;
}


static void get_options(const int argc, const char** argv, struct options* opts)
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


static int start_server(const struct options* opts)
{
  int server_fd, client_fd, in_bytes, out_bytes;
  struct sockaddr_in server_addr, client_addr;
  int addr_len = sizeof(struct sockaddr_in);
  char in_buffer[1024] = {0}, ip_str[20] = {0};

  memset(&server_addr, 0, addr_len);
  memset(&client_addr, 0, addr_len);
  
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = opts->server_port;
  server_addr.sin_addr.s_addr = opts->server_ip;

  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
  {
    perror("start_server(): Failed to create socket");
    exit(EXIT_FAILURE);
  }

  if (bind(server_fd, (struct sockaddr*) &server_addr, addr_len) < 0)
  {
    perror("start_server(): Failed to bind socket to address");
    exit(EXIT_FAILURE);
  }

  if (listen(server_fd, opts->max_clients) < 0)
  {
    perror("start_server(): Failed to designate server socket");
    exit(EXIT_FAILURE);
  }

  printf("Listening on port %d...\n", ntohs(opts->server_port));
  while (1)
  {
    if ((client_fd = accept(server_fd, (struct sockaddr*) &client_addr, &addr_len)) < 0)
    {
      perror("start_server(): Failed to await connections");
      exit(EXIT_FAILURE);
    }
    
    if (inet_ntop(client_addr.sin_family, &(client_addr.sin_addr), ip_str, sizeof(ip_str)) == NULL)
    {
      perror("start_server(): Failed to read client address");
      exit(EXIT_FAILURE);
    }
    printf("Connected to %s.\n", ip_str);

    if ((in_bytes = recv(client_fd, in_buffer, sizeof(in_buffer), 0)) == -1)
    {
      perror("start_server(): Error while reading message from client");
      exit(EXIT_FAILURE);
    }
    if (in_bytes > 0)
    {
      printf("Message from %s:\n%s\n", ip_str, in_buffer);
    }


    close(client_fd);
    memset(&client_addr, 0, sizeof(client_addr));
    memset(ip_str, 0, sizeof(ip_str));
    memset(in_buffer, 0, sizeof(in_buffer));
  }
}


bool header_done(char* buffer, size_t size, int* head_len)
{
  int i = 0;
  for (; i < size-1; i++)
    if (buffer[i] == '\n' && buffer[i+1] == '\n')
    {
      *head_len = i;
      return true;
    }
  return false;
}


int get_content_len(char* buffer, size_t size)
{
  char* temp = buffer;
  char cont_len_key[] = "Content-Length:";
  int key_len = sizeof(cont_len_key);

  int content_len = -1;

  while ((int) temp - buffer < size)
  {
    if (strncmp(cont_len_key, temp, key_len) == 0)
    {
      sprintf(temp+key_len, "%d", &content_len);
      break;
    }
    temp++;
  }
  return content_len;
}


char* receive_message(int client_fd, struct sockaddr_in* client_addr)
{
  char* msg_buffer = calloc(1, BUFFERLEN);
  bool end_of_header = false;
  bool end_of_content = false;
  int total_bytes = 0, head_len = 0, cont_len = 0;
  int max_tries = 10, curr_buff_len = BUFFERLEN;

  while (--max_tries)
  {
    if ((total_bytes += recv(client_fd, msg_buffer, BUFFERLEN-1, 0)) == -1)
    {
      perror("receive_message(): error while reading message from client");
      exit(EXIT_FAILURE);
    }

    /* have we gotten the entire header? */
    if (header_done(msg_buffer, curr_buff_len, &head_len))
      break;

    /* if not, double the buffer size */
    int new_buff_len = curr_buff_len * 2;
    msg_buffer = realloc(msg_buffer, new_buff_len);
    memset((uint64_t) msg_buffer + curr_buff_len, 0, curr_buff_len); /* trailing zeroes */
    curr_buff_len = new_buff_len;
  }

  max_tries = 10;
  cont_len = parse_content_len(

  while (--max_tries)
  {
    if ((
  }

  return msg_buffer;
}


