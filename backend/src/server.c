
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
#include <senderr.h>
#include <client_manager.h>
#include <sql_manager.h>
#include <token_manager.h>
#include <http_post.h>
#include <http_get.h>
#include <http_put.h>
#include <http_delete.h>
#include <server.h>


int main(int argc, char** argv)
{
  init_client_manager();
  init_sql_manager();
  
  struct options opts;
  memset(&opts, 0, sizeof(opts));

  get_options(argc, argv, &opts);

  serve(&opts);

  return 0;
}


void usage(char* name)
{
  fprintf(stderr, "usage: %s [-a <server address>] [-c <max clients] [-p <port>]\n", name);
} 

static void get_options(const int argc, char* const* argv, struct options* opts)
{
  // defaults
  opts->server_port = htons(80);
  opts->max_clients = 10;
  // default ip is already set to 0.0.0.0

  int c, port;
  struct sockaddr_in sa;
  while ((c = getopt(argc, argv, "a:c:p:")) != -1)
  {
    switch(c)
    {
    case 'p':
      port = strtol(optarg, NULL, 10);
      if (port < 1 || port > 65535)
      {
        usage(argv[0]);
        abort();
      }
      opts->server_port = htons(port);
      break;
    case 'c':
      opts->max_clients = strtol(optarg, NULL, 10);
    case 'a':
      if (inet_pton(AF_INET, optarg, &(sa.sin_addr)) == 0) 
      {
        fprintf(stderr, "Invalid server IP address: %s\n", optarg);
        abort();
      }
      opts->server_ip = sa.sin_addr.s_addr;
      break;
    case '?':
    default:
      usage(argv[0]);
      abort();
    }
  }

  if (optind < argc && argv[optind])
  {
    fprintf(stderr, "%s: unknown argument -- '%s'\n", argv[0], argv[optind]);
    usage(argv[0]);
    abort();
  }
}


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

    //terminate_client_manager();
    //terminate_sql_manager();

    //return;
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
    free(message_buffer);
    printf("Connection to %s closed (socket no. %d).\n", ip, client_fd);
    return NULL;
  }

  return message_buffer;
}


static enum request_method get_request_method(char* req_str)
{
  if (req_str == NULL)
  {
    return NO_REQUEST_METHOD;
  }

  char* endofline = strstr(req_str, "\n");

  char* loc = strstr(req_str, "GET");
  if (loc && loc < endofline)
  {
    return GET_REQ;
  }

  loc = strstr(req_str, "HEAD");
  if (loc && loc < endofline)
  {
    return HEAD_REQ;
  }

  loc = strstr(req_str, "PUT");
  if (loc && loc < endofline)
  {
    return PUT_REQ;
  }

  loc = strstr(req_str, "POST");
  if (loc && loc < endofline)
  {
    return POST_REQ;
  }

  loc = strstr(req_str, "DELETE");
  if (loc && loc < endofline)
  {
    return DELETE_REQ;
  }

  return NO_REQUEST_METHOD;
}


static char* get_uri(enum request_method method, char* request)
{
  char* uri = calloc(1, MAXURILEN);
  uri[0] = '.';

  char* getloc;
  if (method == GET_REQ)
  {
    getloc = strstr(request, "GET");
    sscanf(getloc + 4, "%s", uri + 1);
  }
  else if (method == HEAD_REQ)
  {
    getloc = strstr(request, "HEAD");
    sscanf(getloc + 5, "%s", uri + 1);
  }
  else if (method == PUT_REQ)
  {
    getloc = strstr(request, "PUT");
    sscanf(getloc + 4, "%s", uri + 1);
  }
  else if (method == POST_REQ)
  {
    getloc = strstr(request, "POST");
    sscanf(getloc + 5, "%s", uri + 1);
  }
  else /* DELETE_REQ */
  {
    getloc = strstr(request, "DELETE");
    sscanf(getloc + 6, "%s", uri + 1);
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
  char* token_loc = strstr(req_str, find);
  if (token_loc == NULL)
  {
    return NULL;
  }
  token_loc += strlen(find);

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
  if (*contstart == '\0')
  {
    return NULL;
  }

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


  /* fill in request struct */
  struct request req;
  req.method = get_request_method(req_str);

  req.uri = get_uri(req.method, req_str);
  req.token = get_login_token(req_str);
  req.content = get_request_content(req_str);

  free(req_str);

  struct response* resp;

  /* determine correct request handler */ 
  switch (req.method)
  {
  case GET_REQ:
  case HEAD_REQ:
    resp = http_get(&req);
    break;

  case PUT_REQ:
    resp = http_put(&req);
    break;

  case POST_REQ:
    resp = http_post(&req);
    break;

  case DELETE_REQ:
    resp = http_delete(&req);
    break;

  case NO_REQUEST_METHOD:
  default:

    free(req.uri);
    free(req.token);
    free(req.content);
   
    return senderr(BAD_REQUEST);
  }

  free(req.uri);
  free(req.token);
  free(req.content);

  return resp;
}


static void send_msg(int fd, char* buffer, int msg_len)
{
  if (buffer == NULL) return;
  int total = 0, bytes = 0;

  int ct = 0;

  while (total < msg_len)
  {
    if ((bytes = send(fd, buffer + total, msg_len - total, 0)) == -1)
    {
      perror("send_msg(): error while sending response to client");
      exit(EXIT_FAILURE);
    }
    total += bytes; 
    ct ++;
  }

  //printf("Message sent in %d passes\n", ct);
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
    send_msg(client_fd, resp->content, resp->content_length);

  list_delete(resp->header);
  free(resp->content);
  free(resp);
}

