
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include "falcon.h"


int main(const int argc, const char** argv)
{
  struct options opts;
  memset(&opts, 0, sizeof(opts));

  get_options(argc, argv, &opts);

  start_server(&opts);

  return 0;
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
  char* greeting = "Hello there!";
  char in_buffer[1000] = {0};

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

  while (1)
  {
    printf("Awaiting connections...\n");
    if ((client_fd = accept(server_fd, (struct sockaddr*) &client_addr, &addr_len)) < 0)
    {
      perror("Failed to accept incoming connection");
      exit(-1);
    }

    if ((bytes_read = recv(client_fd, in_buffer, sizeof(in_buffer), 0)) <= 0)
    {
      perror("Connection lost");
      exit(-1);
    }
    printf("Message from client on socket %d:\n%s\n%d bytes read\n", client_fd, in_buffer, bytes_read);
    
    if (send(client_fd, greeting, strlen(greeting), 0) < 0)
    {
      perror("Failed to send message");
      exit(-1);
    }
    printf("Response sent to client.\n");

    close(client_fd);
  }
}


