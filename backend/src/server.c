
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <inttypes.h>
#include <kylestructs.h>

#include <common.h>
#include <client_manager.h>
#include <http_post.h>
#include <http_get.h>
#include <http_put.h>
#include <http_delete.h>
#include <server.h>


int main(int argc, char** argv)
{
  init_client_mgr();
  
  struct options opts;
  memset(&opts, 0, sizeof(opts));

  get_options(argc, (const char**) argv, &opts);

  serve(&opts);

  return 0;
}


static void get_options(const int argc, const char** argv, struct options* opts)
{
  char* usage = "Usage: ./falcon.out [-a address] [-p port] [-c clients]\n";

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
}// end get_options


static void serve(const struct options* opts)
{
  int server_fd, client_fd, active_fd, max_fd; 
  struct sockaddr_in server_addr, client_addr;
  int addr_len = sizeof(struct sockaddr_in);
  char ip_str[20] = {0};
  fd_set readfds;

  memset(&server_addr, 0, addr_len);
  memset(&client_addr, 0, addr_len);
  
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = opts->server_port;
  server_addr.sin_addr.s_addr = opts->server_ip;

  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
  {
    perror("serve(): Failed to create socket");
    exit(EXIT_FAILURE);
  }

  if (bind(server_fd, (struct sockaddr*) &server_addr, addr_len) < 0)
  {
    perror("serve(): Failed to bind socket to address");
    exit(EXIT_FAILURE);
  }

  if (listen(server_fd, opts->max_clients) < 0)
  {
    perror("serve(): Failed to designate server socket");
    exit(EXIT_FAILURE);
  }

  /* server socket is first in client socket list */
  add_client(server_fd);

  printf("Listening on port %d...\n", ntohs(opts->server_port));
  while (1)
  {
    
    max_fd = initialize_fdset(&readfds);

    if (select(max_fd+1, &readfds, NULL, NULL, NULL) <= 0)
    {
      perror("server(): failed to wait for socket activity");
      exit(EXIT_FAILURE);
    }

    if ((active_fd = get_active_client(&readfds)) == -1)
    {
      fprintf(stderr, "serve(): no active fds\n");
      exit(EXIT_FAILURE); 
    }

    if (active_fd == server_fd)
    {
      if ((client_fd = accept(server_fd, (struct sockaddr*) &client_addr, &addr_len)) < 0)
      {
        perror("serve(): Failed to accept connection");
        exit(EXIT_FAILURE);
      }
      if (inet_ntop(client_addr.sin_family, &(client_addr.sin_addr), ip_str, sizeof(ip_str)) == NULL)
      {
        perror("serve(): Failed to read client address");
        exit(EXIT_FAILURE);
      }

      /* set recv() timeout to 1s for all client sockets */
      struct timeval tv;
      tv.tv_sec = 1;
      tv.tv_usec = 0;

      if (setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, (const void*)&tv, sizeof(tv))) {
        perror("serve(): Failed on call to setsockopt()");
        exit(EXIT_FAILURE);
      }
      printf("Connected to %s (socket no. %d).\n", ip_str, client_fd);
      add_client(client_fd);
      active_fd = client_fd;
    }
    
    else
    {
      if (getpeername(active_fd, &client_addr, &addr_len) == -1)
      {
        perror("serve(): getpeername() failed");
	exit(EXIT_FAILURE);
      }
      if (inet_ntop(client_addr.sin_family, &(client_addr.sin_addr), ip_str, sizeof(ip_str)) == NULL)
      {
        perror("serve(): Failed to read client address");
        exit(EXIT_FAILURE);
      }
    }

    send_response(
      process_request(
        receive_request(active_fd, ip_str)), active_fd);

    memset(&client_addr, 0, sizeof(client_addr));
    memset(ip_str, 0, sizeof(ip_str));
  }
}


static char* receive_request(int client_fd, char* ip)
{
  char* message_buffer = calloc(1, BUFFERLEN);
  char* curr_buff_ptr = message_buffer;
  int multiplier = 1;
  int total_bytes = 0, bytes = 0, cont_bytes = 0;

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
      if (errno == EAGAIN)
      {
        break;
      }

      perror("receive_request(): error while reading message from client");
      exit(EXIT_FAILURE);
    }
    total_bytes += bytes;
    if (bytes == 0 || bytes < BUFFERLEN - 1) break;

    message_buffer = realloc(message_buffer, BUFFERLEN * multiplier);
    curr_buff_ptr = message_buffer + strlen(message_buffer);
    memset(curr_buff_ptr, 0, BUFFERLEN);
  }

  if (total_bytes == 0)
  {
    remove_client(client_fd);
    close(client_fd);
    free(message_buffer);
    printf("Connection to %s closed (socket no. %d).\n", ip, client_fd);
    return NULL;
  }

  return message_buffer;
}


static enum request_method get_request_method(char* req_str)
{
  if (req_str == NULL) return BAD_REQ;

  char* endofline = strstr(req_str, "\n");

  char* getloc = strstr(req_str, "GET");
  if (getloc && getloc < endofline) return GET_REQ;

  char* headloc = strstr(req_str, "HEAD");
  if (headloc && headloc < endofline) return HEAD_REQ;

  char* putloc = strstr(req_str, "PUT");
  if (putloc && putloc < endofline) return PUT_REQ;

  char* postloc = strstr(req_str, "POST");
  if (postloc && postloc < endofline) return POST_REQ;

  char* delloc = strstr(req_str, "DELETE");
  if (delloc && delloc < endofline) return DELETE_REQ;

  return BAD_REQ;
}


static char* get_login_token(char* req_str)
{
  if (req_str == NULL)
  {
    return NULL;
  }

  char* token_loc = strstr(
}


static char* get_request_content(char* req_str) 
{
  if (req_str == NULL)
  {
    return NULL;
  }

  char* contstart = strstr(req_str, "\r\n\r\n");

  if (contstart == NULL)
  {
    return NULL;
  }

  contstart += 4; // skip over "\r\n\r\n"

  int len = req_str - (contstart - req_str);
  char* content = malloc( len + 1);
  memcpy(content, contstart, len);
  content[len] = '\0';

  return content;
}


static struct response* process_request(char* req_str) 
{ 
  if (req_str == NULL) return NULL; 
  printf("\n%s\n", req_str); 

  /* fill in request struct */
  struct request req;
  req.method = get_request_method(req_str);
  req.login_token = get_login_token(req_str);
  req.content = get_request_content(req_str);
  free(req_str);
 
  if (req.method == GET_REQ || req.method == HEAD_REQ)
    return http_get(req);

  if (req.method == PUT_REQ)
    return http_put(req);

  if (req.method == POST_REQ)
    return http_post(req);

  if (req.method == DELETE_REQ)
    return http_delete(req);

  free(req.login_token);
  free(req.content);

  fprintf(stderr, "process_request(): 400 Bad Request\n");

  /* Set up bad request response */
  char* h1 = STAT400;
  char* h2 = "Connection: close\n";
  char* h3 = "Content-Length: 0\n";

  struct response* resp = calloc(1, sizeof(struct response));
  resp->header = list_new();

  list_add(resp->header, datacont_new(h1, CHARP, strlen(h1)));
  list_add(resp->header, datacont_new(h2, CHARP, strlen(h2)));
  list_add(resp->header, datacont_new(h3, CHARP, strlen(h3)));

  return resp;
}


void send_msg(int fd, char* buffer, int msg_len)
{
  if (buffer == NULL) return;
  int total = 0, bytes = 0;

  while (total < msg_len)
  {
    if ((bytes = send(fd, buffer + total, msg_len - total, 0)) == -1)
    {
      perror("send_msg(): error while sending response to client");
      exit(EXIT_FAILURE);
    }
    total += bytes; 
  }
}


static void send_response(struct response* resp, int client_fd)
{
  if (resp == NULL)
    return;

  int total, bytes, msg_len;
  int head_lines = list_length(resp->header);

  printf("Sending response:\n\n"); 
  for (int i = 0; i < head_lines; i++)
  {
    datacont* line = list_get(resp->header, i);
    printf("%s", line->cp);
    send_msg(client_fd, line->cp, line->size);
  }
  printf("\n");

  send_msg(client_fd, "\n", 1);
  if (resp->content)
    send_msg(client_fd, resp->content->cp, resp->content->size);

  list_delete(resp->header);
  datacont_delete(resp->content);
  free(resp);
}

