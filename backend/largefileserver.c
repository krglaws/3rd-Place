
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

char* receive_message(int, struct sockaddr_in*);



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


static void start_server(const struct options* opts)
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

    char* message = receive_message(client_fd, &client_addr);

    printf("Message from client %s:\n%s\n", ip_str, message);
    free(message);
/*
    if ((in_bytes = recv(client_fd, in_buffer, sizeof(in_buffer), 0)) == -1)
    {
      perror("start_server(): Error while reading message from client");
      exit(EXIT_FAILURE);
    }
    if (in_bytes > 0)
    {
      printf("Message from %s:\n%s\n", ip_str, in_buffer);
    }
*/
    close(client_fd);
    memset(&client_addr, 0, sizeof(client_addr));
    memset(ip_str, 0, sizeof(ip_str));
    memset(in_buffer, 0, sizeof(in_buffer));
  }
}


int header_done(char* buffer, size_t size, int* head_len)
{
  char terminator[] = "\r\n\r\n";
  char* end_of_header = strstr(buffer, terminator);
  
  for (int i = 0; i < size - sizeof(terminator); i++)
  {
    for (int j = i; j < i+4; j++) printf("%c", buffer[j]);
    printf("\n");
    if (strncmp(terminator, buffer+i, sizeof(terminator)) == 0)
    {
      printf("end of header detected\n");
      *head_len = i + 1;
      return 1;
    }
  }
  return 0;
}


int get_content_len(char* buffer)
{
  char* temp = buffer;
  char* cont_len_key = "Content-Length:";

  int content_len = 0;
  if ((temp = strstr(buffer, cont_len_key)) != NULL)
    sscanf(temp, "%*s %d", &content_len);

  return content_len;
}

/* FIX THIS TO CANCEL THE REQUEST IF THE HEADER IS TOO BIG. RIGHT NOW
THE ONLY CUT OFF IS ONCE THE BUFFER IS COMPLETELY FULL, AND THERE IS NO ERROR. */
char* receive_message(int client_fd, struct sockaddr_in* client_addr)
{
  char* message_buffer = calloc(1, BUFFERLEN+1);
  char* curr_buff_ptr = message_buffer;
  char* header_end;
  int bytes, total_bytes = 0, head_bytes = 0, cont_bytes = 0,
    tries = 0, curr_buff_len = BUFFERLEN, buffer_multiplier = 1;

  while (tries++ < 10)
  {
    if ((bytes = recv(client_fd, curr_buff_ptr, BUFFERLEN - 1, 0)) == -1)
    {
      perror("receive_message(): error while reading message from client");
      exit(EXIT_FAILURE);
    }
    total_bytes += bytes;
    printf("received %d bytes\n", bytes);

    /* have we gotten the entire header? */
    
    if ((header_end = strstr(message_buffer, "\r\n\r\n")) != NULL)
    {
      head_bytes = (header_end+4) - message_buffer;
      break;
    }

    /* if not, increase buffer size by BUFFERLEN bytes */
    buffer_multiplier++;
    message_buffer = realloc(message_buffer, BUFFERLEN * buffer_multiplier);
    curr_buff_ptr = message_buffer + strlen(message_buffer);
    memset(curr_buff_ptr, 0, BUFFERLEN); /* trailing zeroes */
  }

  /* check content length */
  cont_bytes = get_content_len(message_buffer);

  /* if none is specified, return */
  if (cont_bytes == 0)
    return message_buffer;

  /* if we have received whole message, return */
  if (total_bytes <= head_bytes + cont_bytes)
    return message_buffer;

  /* otherwise, check if the message is too big */
  else if (buffer_multiplier > 10)
  {
    fprintf(stderr, "receive_message(): client message too big\n");
    exit(EXIT_FAILURE);
  }

  /* read in rest of body */
  while (tries++ < 10)
  {
    if ((bytes = recv(client_fd, curr_buff_ptr, BUFFERLEN - 1, 0)) == -1)
    {
      perror("receive_message(): error while reading message from client");
      exit(EXIT_FAILURE);
    }
    total_bytes += bytes;

    /* check if we have whole message */
    if (total_bytes <= head_bytes + cont_bytes)
      break;

    /* if not, increase buffer size by BUFFERLEN bytes */
    buffer_multiplier++;
    message_buffer = realloc(message_buffer, BUFFERLEN * buffer_multiplier);
    curr_buff_ptr = message_buffer + strlen(message_buffer);
    memset(curr_buff_ptr, 0, BUFFERLEN); /* trailing zeroes */
  }

  /* if we have received whole message, return */
  if (total_bytes <= head_bytes + total_bytes)
    return message_buffer;
  
  /* otherwise, check if the message is too big */
  else if (buffer_multiplier > 10)
  {
    fprintf(stderr, "receive_message(): client message too big\n");
    exit(EXIT_FAILURE);
  }

  fprintf(stderr, "Something went wrong");
  return NULL;

  //return msg_buffer;
}


