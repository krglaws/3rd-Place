#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <inttypes.h>
#include <kylestructs.h>

#include <common.h>
#include <senderr.h>
#include <log_manager.h>
#include <socket_manager.h>
#include <sql_manager.h>
#include <auth_manager.h>
#include <http_post.h>
#include <http_get.h>
#include <http_put.h>
#include <http_delete.h>
#include <server.h>


int main(int argc, char** argv)
{
  // process CLI args
  struct options* opts = calloc(1, sizeof(struct options));
  get_options(argc, argv, opts);

  // set things up
  init_server(opts);

  // enter server loop
  serve();

  return -1;
}


void usage(const char* name)
{
  fprintf(stderr, "usage: %s [-a <server address>] [-c <max clients] [-p <port>]\n", name);
}
 

static void get_options(const int argc, char* const* argv, struct options* opts)
{
  // defaults
  opts->server_addr = DEFAULT_ADDR;
  opts->server_port = htons(DEFAULT_PORT);
  opts->max_clients = DEFAULT_MAX_CLIENTS;
  // default logpath is already set to NULL (stdout)

  int c, port;
  int logpathlen;
  struct in6_addr ipv6addr;
  while ((c = getopt(argc, argv, "a:c:p:f:")) != -1)
  {
    switch(c)
    {
    case 'p':
      port = strtol(optarg, NULL, 10);
      if (port < 1 || port > 65535)
      {
        fprintf(stderr, "Invalid port: %d", port);
        delete_options(opts);
        exit(EXIT_FAILURE);
      }
      opts->server_port = htons(port);
      break;
    case 'f':
      logpathlen = strlen(optarg);
      opts->logpath = malloc((logpathlen * sizeof(char)) + 1);
      memcpy(opts->logpath, optarg, logpathlen + 1);
      break;
    case 'c':
      opts->max_clients = strtol(optarg, NULL, 10);
      break;
    case 'a':
      if (inet_pton(AF_INET6, optarg, &ipv6addr) != 1) 
      {
        fprintf(stderr, "Invalid server IPv6 address: %s\n", optarg);
        delete_options(opts);
        exit(EXIT_FAILURE);
      }
      opts->server_addr = ipv6addr;
      break;
    case '?':
    default:
      usage(argv[0]);
      delete_options(opts);
      exit(EXIT_FAILURE);
    }
  }

  if (optind < argc && argv[optind])
  {
    fprintf(stderr, "%s: unknown argument -- '%s'\n", argv[0], argv[optind]);
    usage(argv[0]);
    delete_options(opts);
    exit(EXIT_FAILURE);
  }
}


void delete_options(struct options* opts)
{
  if (opts->logpath != NULL)
  {
    free(opts->logpath);
  }

  free(opts);
}


static void init_server(struct options* opts)
{
  // first init logger (since others use the logger)
  init_log_manager(opts->logpath);

  // set signal handler
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = terminate_server;

  sigaction(SIGTERM, &sa, NULL);
  sigaction(SIGINT, &sa, NULL);

  // init remaining services
  init_auth_manager();

  init_sql_manager();

  init_socket_manager(&(opts->server_addr), opts->server_port, opts->max_clients);

  delete_options(opts);
}


void terminate_server(const int signum)
{
  // terminate services
  terminate_socket_manager();

  terminate_sql_manager();

  terminate_auth_manager();

  terminate_log_manager();

  if (signum == SIGINT)
  {
    // assuming we received SIGINT (Ctrl-C) from terminal
    exit(EXIT_SUCCESS);
  }

  // any other signal, assume the worst
  exit(EXIT_FAILURE);
}


static void serve()
{
  while (1)
  {
    int active_socket = await_active_socket();

    send_response(active_socket,
      process_request(active_socket,
        receive_request(active_socket)));
  }
}


static char* receive_request(const int sock)
{
  char* message_buffer = calloc(1, BUFFERLEN);
  char* curr_buff_ptr = message_buffer;
  int multiplier = 1;
  int total_bytes = 0, bytes = 0, cont_bytes = 0;

  while (multiplier++ < MAXBUFFERMUL+1)
  {
    // check if message is too big
    if (multiplier >= MAXBUFFERMUL)
    {
      log_err("receive_request(): Request exceeds %d byte limit", (MAXBUFFERMUL * BUFFERLEN));
      free(message_buffer);
      return NULL;
    }

    // read in bytes from socket
    if ((bytes = recv(sock, curr_buff_ptr, BUFFERLEN - 1, 0)) == -1)
    {
      log_err("receive_request(): recv(): %s ", strerror(errno));
      remove_socket(sock);
      free(message_buffer);
      return NULL;
    }

    // increment total bytes read
    total_bytes += bytes;
    if (bytes == 0 || bytes < BUFFERLEN - 1)
    {
      break;
    }

    // adjust message buffer
    message_buffer = realloc(message_buffer, BUFFERLEN * multiplier);
    curr_buff_ptr = message_buffer + strlen(message_buffer);
    memset(curr_buff_ptr, 0, BUFFERLEN);
  }

  // 0 bytes read means client disconnected
  if (total_bytes == 0)
  {
    remove_socket(sock);
    free(message_buffer);
    return NULL;
  }

  return message_buffer;
}


static enum request_method get_request_method(const char* req_str)
{
  if (req_str == NULL)
  {
    return BAD_REQ;
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

  return BAD_REQ;
}


static char* get_uri(const enum request_method method, const char* request)
{
  if (strstr(request, "..") || method == BAD_REQ)
  {
    return NULL;
  }

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
  else //if (method == DELETE_REQ)
  {
    getloc = strstr(request, "DELETE");
    sscanf(getloc + 6, "%s", uri + 1);
  }

  return uri;
}


static const struct auth_token* get_login_token(const char* req_str)
{
  if (req_str == NULL)
  {
    return NULL;
  }

  // locate cookie location within request string
  char* find = "Cookie: logintoken=";
  char* token_loc = strstr(req_str, find);
  if (token_loc == NULL)
  {
    return NULL;
  }
  token_loc += strlen(find);

  char* newline = strstr(token_loc, "\r\n");
  int len = newline - token_loc;

  if (len != TOKENLEN)
  {
    return NULL;
  }

  // copy token
  char tokenstr[TOKENLEN + 1];
  memcpy(tokenstr, token_loc, TOKENLEN);
  tokenstr[TOKENLEN] = '\0';

  const struct auth_token* token = valid_token(tokenstr);

  return token;
}


static char* get_request_content(const char* req_str) 
{
  if (req_str == NULL)
  {
    return NULL;
  }

  // find end of request header
  char* contstart = strstr(req_str, "\r\n\r\n");

  if (contstart == NULL)
  {
    return NULL;
  }

  contstart += 4; // skip over '\r\n\r\n'
  if (*contstart == '\0')
  {
    return NULL;
  }

  int len = strlen(contstart);
  char* content = malloc(sizeof(char) * (len + 1));
  memcpy(content, contstart, len);
  content[len] = '\0';

  return content;
}


static struct response* process_request(const int sock, char* req_str) 
{ 
  if (req_str == NULL)
  {
    // client disconnected
    return NULL; 
  }

  // fill in request struct
  struct request req;
  req.content = get_request_content(req_str);
  req.method = get_request_method(req_str);
  req.uri = get_uri(req.method, req_str);
  req.client_info = get_login_token(req_str);

  // check if method could be found, but URI was bad
  if (req.method != BAD_REQ && req.uri == NULL)
  {
    req.method = BAD_REQ;
  }

  free(req_str);

  // get socket info
  char ipstr[64];
  if (get_socket_ip(sock, ipstr, sizeof(ipstr)) == -1)
  {
    log_err("process_request(): failed on call to get_socket_ip()");
    remove_socket(sock);
    return NULL;
  }

  struct response* resp;

  // determine correct request handler
  switch (req.method)
  {
  case GET_REQ:
    log_info("Request from %s: GET %s", ipstr, (req.uri + 1));
    resp = http_get(&req);
    break;

  case HEAD_REQ:
    log_info("Request from %s: HEAD %s", ipstr, (req.uri + 1));
    resp = http_get(&req);
    break;

  case PUT_REQ:
    log_info("Request from %s: PUT %s", ipstr, (req.uri + 1));
    resp = http_put(&req);
    break;

  case POST_REQ:
    log_info("Request from %s: POST %s", ipstr, (req.uri + 1));
    resp = http_post(&req);
    break;

  case DELETE_REQ:
    log_info("Request from %s: DELETE %s", ipstr, (req.uri + 1));
    resp = http_delete(&req);
    break;

  case BAD_REQ:
  default:
    log_info("Bad request from %s", ipstr);

    free(req.uri);
    free(req.content);

    return senderr(ERR_BAD_REQ);
  }

  free(req.uri);
  free(req.content);

  return resp;
}


static int send_msg(const int sock, char* buffer, int msg_len)
{
  if (buffer == NULL)
  {
    return -1;
  }

  int total = 0, bytes = 0;

  while (total < msg_len)
  {
    if ((bytes = send(sock, buffer + total, msg_len - total, 0)) == -1)
    {
      log_err("send_msg(): send(): %s", strerror(errno));
      remove_socket(sock);
      return -1;
    }
    total += bytes; 
  }

  return 0;
}


static void delete_response(struct response* resp)
{
  list_delete(resp->header);
  free(resp->content);
  free(resp);
}


static void send_response(const int sock, struct response* resp)
{
  if (resp == NULL)
  {
    // client disconnected
    return;
  }

  // log first line of resp header
  log_info("Response: %s", list_get(resp->header, 0)->cp);

  // send each line of header
  int total, bytes, msg_len;
  int head_lines = list_length(resp->header);
  for (int i = 0; i < head_lines; i++)
  {
    datacont* line = list_get(resp->header, i);
    if (send_msg(sock, line->cp, line->size) == -1)
    {
      delete_response(resp);
      return;
    }
  }

  // end header
  send_msg(sock, "\n", 1);

  // send resp body
  if (resp->content)
  {
    send_msg(sock, resp->content, resp->content_length);
  }

  delete_response(resp);
}
