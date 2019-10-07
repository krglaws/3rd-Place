
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "falcon.h"


int main(int argc, char** argv)
{
  struct options* opts = get_options(argc, argv);

  

  return 0;
}


struct options* get_options(int argc, char** argv)
{
  struct options* opts = calloc(1, sizeof(struct options));

  char* usage = "Usage: ./FalconServer [-p port] [-c clients]\n";

  for (int i = 1; i < argc; i++)
  {
    if (strcmp("-p", argv[i]) == 0)
    {
      int port = 0;
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
      opts->port = port;
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
      opts->clients = clients;
      i++;
    }
    else
    {
      fprintf(stderr, "Unknown option: %s\n%s", argv[i], usage);
      exit(-1);
    }
  }

  // defaults
  if (opts->port == 0) opts->port = 80;
  if (opts->clients == 0) opts->clients = 10;

  return opts;
}// end valid_args


void start_server(const struct options* opts)
{
  int server_fd;
  struct sockaddr_in address;

  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    perror("Failed to create socket");
    exit(-1);
  }
  memset(&address, 0, sizeof(struct sockaddr_in));
  
  address.sin_family = AF_INET;
  address.sin_port = opts->port;
  address.sin_addr = INADDR_ANY;
  


  memset(&server_address, 
}


