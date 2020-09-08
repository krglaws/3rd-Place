
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
#include <token_manager.h>
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


static char* get_uri(enum request_method method, char* request)
{
  char* uri = calloc(1, MAXURILEN);
  uri[0] = '.';

  char* getloc;
  if (method == GET_REQ)
  {
    sscanf(getloc + 4, "%s", uri + 1);
    getloc = strstr(request, "GET");
  }
  else if (method == HEAD_REQ)
  {
    sscanf(getloc + 5, "%s", uri + 1);
    getloc = strstr(request, "HEAD");
  }
  else if (method == PUT_REQ)
  {
    sscanf(getloc + 4, "%s", uri + 1);
    getloc = strstr(request, "PUT");
  }
  else if (method == POST_REQ)
  {
    sscanf(getloc + 5, "%s", uri + 1);
    getloc = strstr(request, "POST");
  }
  else /* DELETE_REQ */
  {
    sscanf(getloc + 6, "%s", uri + 1);
    getloc = strstr(request, "DELETE");
  }

  if (strcmp(uri, "./") == 0)
  {
    memcpy(uri, "./index.html", strlen("./index.html") + 1);
  }

  return uri;
}


static char* get_login_token(char* req_str)
{
  if (req_str == NULL)
  {
    return NULL;
  }

  /* locate token location within request string */
  char* find = "Cookie: logintoken=";
  char* token_loc = strstr(req_str, find) + strlen(find);
  char* newline = strstr(token_loc, "\n");
  int len = newline - token_loc;

  if (len == 0)
  {
    return NULL;
  }

  /* copy token */
  char* token = malloc(len + 1);
  memcpy(token, token_loc, len);
  token[len] = '\0';

  return token;
}


static char* get_request_content(char* req_str) 
{
  if (req_str == NULL)
  {
    return NULL;
  }

  /* find end of request header */
  char* contstart = strstr(req_str, "\r\n\r\n");

  if (contstart == NULL)
  {
    return NULL;
  }

  contstart += 4; // skip over "\r\n\r\n"

  int len = ((long int)req_str) - (contstart - req_str);
  char* content = malloc( len + 1);
  memcpy(content, contstart, len);
  content[len] = '\0';

  return content;
}


static struct response* process_request(char* req_str) 
{ 
  if (req_str == NULL) return NULL; 
  printf("\n%s\n", req_str);

  struct response* resp;

  /* fill in request struct */
  struct request req;
  req.method = get_request_method(req_str);

  if (req.method == BAD_REQ)
  {
    /* Set up bad request response */
    char* header_str = STAT400 "Connection: close\nContent-Length: 0\n";
    resp = malloc(sizeof(struct response));

    resp->header = list_new();
    resp->content = NULL;
    list_add(resp->header, datacont_new(header_str, CHARP, strlen(header_str)));

    return resp;
  }

  req.uri = get_uri(req.method, req_str);
  req.token = get_login_token(req_str);
  req.content = get_request_content(req_str);

  free(req_str);
 
  if (req.method == GET_REQ || req.method == HEAD_REQ)
    resp = http_get(&req);

  if (req.method == PUT_REQ)
    resp = http_put(&req);

  if (req.method == POST_REQ)
    resp = http_post(&req);

  if (req.method == DELETE_REQ)
    resp = http_delete(&req);

  free(req.uri);
  free(req.token);
  free(req.content);

  return resp;
}


static void send_msg(int fd, char* buffer, int msg_len)
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
    send_msg(client_fd, resp->content, strlen(resp->content));

  list_delete(resp->header);
  free(resp->content);
  free(resp);
}

