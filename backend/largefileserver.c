
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define BUFFERLEN 1024
#define MAXBUFFERMUL 10

struct options
{
  uint16_t server_port;
  uint32_t server_ip;
  int max_clients;
  int max_queue;
};


static void get_options(const int argc, const char** argv, struct options* opts);

static void serve(const struct options* opts);

char* receive_request(int client_fd, struct sockaddr_in* client_addr);

char* process_request(char* request, int client_fd, struct sockaddr_in* client_addr);

char* send_request(char* request, int client_fd, struct sockaddr_in* client_addr);


int main(const int argc, const char** argv)
{
  struct options opts;
  memset(&opts, 0, sizeof(opts));

  get_options(argc, argv, &opts);

  serve(&opts);

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


static void serve(const struct options* opts)
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

    send_response( process_request( receive_request(
      client_fd, &client_addr),
        client_fd, &client_addr)
          client_fd, &client_addr);

    close(client_fd);
    memset(&client_addr, 0, sizeof(client_addr));
    memset(ip_str, 0, sizeof(ip_str));
    memset(in_buffer, 0, sizeof(in_buffer));
  }
}


char* receive_request(int client_fd, struct sockaddr_in* client_addr)
{
  char* message_buffer = calloc(1, BUFFERLEN);
  char* curr_buff_ptr = message_buffer;
  int multiplier = 1;
  int bytes = 0, cont_bytes = 0;
  
  while (multiplier++ < MAXBUFFERMUL+1)
  {
    if (multiplier >= MAXBUFFERMUL)
    {
      fprintf(stderr, "receive_request(): Request exceeds %d byte limit\n", MAXBUFFERMUL * BUFFERLEN);
      free(message_buffer);
      return NULL;
    }
    if ((bytes = recv(client_fd, curr_buff_ptr, BUFFERLEN - 1, 0)) == -1)
    {
      perror("receive_request(): error while reading message from client");
      exit(EXIT_FAILURE);
    }
    if (bytes == 0 || bytes < BUFFERLEN - 1) break;
    
    message_buffer = realloc(message_buffer, BUFFERLEN * multiplier);
    curr_buff_ptr = message_buffer + strlen(message_buffer);
    memset(curr_buff_ptr, 0, BUFFERLEN);
  }

  return message_buffer;
}


char* process_request(char* request, int client_fd, struct sockaddr_in* client_addr)
{
  if (strncmp("PUT", request, 3) == 0)
    return http_put(request);

  else if (strncmp("GET", request, 3) == 0 ||
           strcmp("HEAD", request, 4) == 0)
    return http_get(request);

  else if (strncmp("POST", request, 4) == 0)
    return http_post(request);

  else if (strncmp("DELETE", request, 6) == 0)
    return http_delete(request);
  
  fprintf(stderr, "process_request(): 400 Bad Request\n");

  char* bad_request = "HTTP/1.1 400 Bad Request\nConnection: close\nContent-Length: 0\n\n";
  char* response = malloc(sizeof(bad_request));
  memcpy(response, bad_request, sizeof(bad_request));

  return response;    
}


void send_response(char* response, int client_fd, struct sockaddr_in* client_addr)
{
  int total = 0, bytes = 0, msg_len = strlen(response);

  while (total < msg_len)
  {
    if ((bytes = send(client_fd, response, msg_len, 0)) == -1)
    {
      perror("send_response(): error while sending response to client");
      exit(EXIT_FAILURE);
    }
    total += bytes;
  }
}

