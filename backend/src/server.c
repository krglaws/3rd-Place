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
#include <string_map.h>
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
      process_request(active_socket));
  }
}


#define COPY_STR(dest, s) {\
  int s_len = strlen(s);\
  char* s_cpy = malloc((s_len + 1) * sizeof(char));\
  memcpy(s_cpy, s, s_len);\
  s_cpy[s_len] = '\0';\
  dest = s_cpy;\
}


/* If something is off about the request structure,
 * return NULL and send 400 Bad Request back to client.
 */
static struct request* parse_request(char* req_buf)
{
  // delimiters
  char* space_del = " ";
  char* question_del = "?";
  char* crlf_del = "\r\n";
  char* hdrend_del = "\r\n\r\n";

  // grab first line of request
  char* end_req_line;
  if ((end_req_line = strstr(req_buf, crlf_del)) == NULL)
  {
    return NULL;
  }

  struct request* req = calloc(1, sizeof(struct request));

  // NULL terminate request line
  *end_req_line = '\0';
  end_req_line += 2;

  // get request method
  char* token;
  if ((token = strtok(req_buf, space_del)) == NULL)
  {
    delete_request(req);
    return NULL;
  }
  COPY_STR(req->method, token);

  // get URI
  if ((token = strtok(NULL, space_del)) == NULL)
  {
    delete_request(req);
    return NULL;
  }

  // check for query string
  char* q_begin;
  if ((q_begin = strstr(token, "?")) != NULL)
  {
    // delete '?'
    *q_begin = '\0';
    q_begin++;
    req->query = string_to_map(q_begin, "&", "=");
  }

  // prep uri string and copy
  int uri_len = strlen(token);
  req->uri = malloc(sizeof(char) * (strlen(token) + 2));
  req->uri[0] = '.'; // prepend '.'
  memcpy(req->uri+1, token, uri_len);
  req->uri[uri_len+1] = '\0';

  // get HTTP version
  if ((token = strtok(NULL, space_del)) == NULL)
  {
    delete_request(req);
    return NULL;
  }
  COPY_STR(req->http_version, token);

  // find end of header
  char* hdr_end;
  if ((hdr_end = strstr(end_req_line, "\r\n\r\n")) == NULL)
  {
    delete_request(req);
    return NULL;
  }

  *hdr_end = '\0';
  hdr_end += 4;

  // parse header and content
  req->header = string_to_map(end_req_line, crlf_del, ": ");
  req->content = string_to_map(hdr_end, "&", "=");

  // parse cookies
  const ks_datacont* cookies;
  if ((cookies = get_map_value(req->header, "Cookie")) != NULL)
  {
    char* cookies_cpy;
    COPY_STR(cookies_cpy, cookies->cp);
    req->cookies = string_to_map(cookies_cpy, "; ", "=");
    free(cookies_cpy);
  }

  // get client info
  const ks_datacont* login_token;
  if ((login_token = get_map_value(req->cookies, "logintoken")) != NULL)
  {
    req->client_info = (struct auth_token* ) valid_token(login_token->cp);
  }

  return req;
}


static void delete_request(struct request* req)
{
  free(req->http_version);
  free(req->method);
  free(req->uri);
  ks_hashmap_delete(req->query);
  ks_hashmap_delete(req->header);
  ks_hashmap_delete(req->cookies);
  ks_hashmap_delete(req->content);
  free(req);
}


static struct response* process_request(const int sock)
{
  char ipstr[64];
  get_socket_ip(sock, ipstr, 64);

  // stack allocate request buffer here
  char req_buf[MAXREQUESTSIZE + 1];
  req_buf[0] = '\0';

  // read request into buffer
  int req_len;
  if ((req_len = recv(sock, req_buf, MAXREQUESTSIZE + 1, 0)) == -1)
  {
    log_err("process_request(): Failed on call to recv(): %s", strerror(errno));
    remove_socket(sock);
    return NULL;
  }

  if (req_len == 0)
  {
    log_info("Connection to %s closed (sock no. %d)", ipstr, sock);
    remove_socket(sock);
    return NULL;
  }

  if (req_len > MAXREQUESTSIZE)
  {
    log_info("Bad request from %s (sock no. %d)", ipstr, sock);
    return senderr(ERR_MSG_TOO_BIG);
  }

  // pass the buffer to parse_request()
  struct request* req;
  if ((req = parse_request(req_buf)) == NULL)
  {
    return senderr(ERR_BAD_REQ);
  }

  struct response* resp;

  // send request object to appropriate handler
  if (strcmp(req->method, "GET") == 0 ||
           strcmp(req->method, "HEAD") == 0)
  {
    log_info("Request from %s (sock no. %d): %s %s", ipstr, sock, req->method, (req->uri + 1));
    resp = http_get(req);
  }
  else if (strcmp(req->method, "POST") == 0)
  {
    log_info("Request from %s(sock no. %d): POST %s", ipstr, sock, (req->uri + 1));
    resp = http_post(req);
  }
  else if (strcmp(req->method, "PUT") == 0)
  {
    log_info("Request from %s(sock no. %d): PUT %s", ipstr, sock, (req->uri + 1));
    resp = http_put(req);
  }
  else if (strcmp(req->method, "DELETE") == 0)
  {
    log_info("Request from %s(sock no. %d): DELETE %s", ipstr, sock, (req->uri + 1));
    resp = http_delete(req);
  }
  else
  {
    log_info("Bad request from %s", ipstr);
    resp = senderr(ERR_BAD_REQ);
  }

  delete_request(req);

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
  ks_list_delete(resp->header);
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
  log_info("Response: %s", ks_list_get(resp->header, 0)->cp);

  // send each line of header
  int total, bytes, msg_len;
  int head_lines = ks_list_length(resp->header);
  for (int i = 0; i < head_lines; i++)
  {
    ks_datacont* line = ks_list_get(resp->header, i);
    if (send_msg(sock, line->cp, line->size) == -1)
    {
      delete_response(resp);
      return;
    }
  }

  // end header
  send_msg(sock, "\r\n", 2);

  // send resp body
  if (resp->content)
  {
    send_msg(sock, resp->content, resp->content_length);
  }

  delete_response(resp);
}
