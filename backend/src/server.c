#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <inttypes.h>
#include <kylestructs.h>

#include <response.h>
#include <string_map.h>
#include <file_manager.h>
#include <log_manager.h>
#include <socket_manager.h>
#include <sql_manager.h>
#include <auth_manager.h>
#include <http_post.h>
#include <http_get.h>
#include <server.h>

/* Fills out an options structure with information from args
   passed via commandline. Uses default options if no args 
   passed. */
static void get_options(const int argc, char* const* argv, struct options* opts);

/* Initializes services, signal handling, and sets up server socket */
static void init_server(struct options* opts);

/* Called when SIGTERM or SIGINT are received */
static void terminate_server(const int signum);

/* Parses the raw request data into a request struct */
static struct request* parse_request(char* req_buf);
static void delete_request(struct request* req);

/* Parses the request string into a 'request' struct, passes it to 
   the corresponding HTTP method handler, and returns a 'response' 
   struct. */
static struct response* process_request(struct sock_entry* se);

/* Sends the contents of a buffer to a specified socket */
static int send_msg(struct sock_entry* se, char* buffer, int msg_len);

/* Delete response object */
static void delete_response(struct response* resp);

/* Uses send_msg() to send the response in pieces */
static void send_response(struct sock_entry* se, struct response* resp);

/* Takes in a structure of execution options and listens for
   incoming connections. Never returns. */
static void serve();


int main(int argc, char** argv)
{
  // process CLI args
  struct options opts;
  memset(&opts, 0, sizeof(opts));
  get_options(argc, argv, &opts);

  // set things up
  init_server(&opts);

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
        exit(EXIT_FAILURE);
      }
      opts->server_port = htons(port);
      break;
    case 'f':
      logpathlen = strlen(optarg);
      if (logpathlen > 128)
      {
        fprintf(stderr, "log file path is too long (> 128 chars)\n");
        exit(EXIT_FAILURE);
      }
      memcpy(opts->logpath, optarg, logpathlen + 1);
      break;
    case 'c':
      opts->max_clients = strtol(optarg, NULL, 10);
      break;
    case 'a':
      if (inet_pton(AF_INET6, optarg, &ipv6addr) != 1) 
      {
        fprintf(stderr, "Invalid server IPv6 address: %s\n", optarg);
        exit(EXIT_FAILURE);
      }
      opts->server_addr = ipv6addr;
      break;
    case '?':
    default:
      usage(argv[0]);
      exit(EXIT_FAILURE);
    }
  }

  if (optind < argc && argv[optind])
  {
    fprintf(stderr, "%s: unknown argument -- '%s'\n", argv[0], argv[optind]);
    usage(argv[0]);
    exit(EXIT_FAILURE);
  }
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

  init_http_post();

  init_http_get();

  init_file_manager();

  init_auth_manager();

  init_sql_manager();

  init_socket_manager(&(opts->server_addr), opts->server_port, opts->max_clients);
}


static void terminate_server(const int signum)
{
  log_info("Shutting down...");

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
    struct sock_entry* active_socket = await_active_socket();

    // skip on error
    if (active_socket == NULL)
    {
      continue;
    }

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
  // grab first line of request
  char* end_req_line;
  if ((end_req_line = strstr(req_buf, "\r\n")) == NULL)
  {
    return NULL;
  }

  struct request* req = calloc(1, sizeof(struct request));

  // NULL terminate request line
  *end_req_line = '\0';
  end_req_line += 2;

  // get request method
  char* token;
  if ((token = strtok(req_buf, " ")) == NULL)
  {
    delete_request(req);
    return NULL;
  }
  COPY_STR(req->method, token);

  // get URI
  if ((token = strtok(NULL, " ")) == NULL)
  {
    delete_request(req);
    return NULL;
  }
  char* uri;
  COPY_STR(uri, token);

  // get HTTP version
  if ((token = strtok(NULL, " ")) == NULL)
  {
    delete_request(req);
    return NULL;
  }
  COPY_STR(req->http_version, token);

  // check for query string
  char* q_begin;
  if ((q_begin = strstr(uri, "?")) != NULL)
  {
    //delete '?'
    *q_begin = '\0';
    q_begin++;
    req->query = string_to_map(q_begin, "&", "=");
  }

  // prep uri string and copy
  int uri_len = strlen(uri);
  req->uri = malloc(sizeof(char) * (uri_len + 2));
  req->uri[0] = '.'; // prepend '.'
  memcpy(req->uri+1, uri, uri_len);
  req->uri[uri_len+1] = '\0';
  free(uri);

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
  req->header = string_to_map(end_req_line, "\r\n", ": ");
  req->content = string_to_map(hdr_end, "&", "=");

  // parse cookies
  const char* cookies;
  if ((cookies = get_map_value_str(req->header, "Cookie")) != NULL)
  {
    char* cookies_cpy;
    COPY_STR(cookies_cpy, cookies);
    req->cookies = string_to_map(cookies_cpy, "; ", "=");
    free(cookies_cpy);
  }

  // get client info
  const char* auth;
  if ((auth = get_map_value_str(req->cookies, "auth")) != NULL)
  {
    req->client_info = (struct auth_token* ) valid_token(auth);
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


static struct response* process_request(struct sock_entry* se)
{
  // stack allocate request buffer here
  char req_buf[MAXREQUESTSIZE + 1];
  memset(req_buf, 0, MAXREQUESTSIZE + 1);

  // read request into buffer
  int req_len;
  if ((req_len = recv(se->sock, req_buf, MAXREQUESTSIZE + 1, 0)) == -1)
  {
    log_err("process_request(): Failed on call to recv(): %s", strerror(errno));
    remove_socket(se);
    return NULL;
  }

  if (req_len == 0)
  {
    remove_socket(se);
    return NULL;
  }

  if (req_len > MAXREQUESTSIZE)
  {
    log_info("Bad request from %s (socket number %d)", se->addr, se->sock);
    return response_error(STAT413);
  }

  // pass the buffer to parse_request()
  struct request* req;
  if ((req = parse_request(req_buf)) == NULL)
  {
    return response_error(STAT400);
  }

  struct response* resp;

  log_info("Request from %s (socket number %d): %s %s", se->addr, se->sock, req->method, (req->uri + 1));

  // send request object to appropriate handler
  if (strcmp(req->method, "GET") == 0 ||
           strcmp(req->method, "HEAD") == 0)
  {
    resp = http_get(req);
  }
  else if (strcmp(req->method, "POST") == 0)
  {
    resp = http_post(req);
  }
  else
  {
    resp = response_error(STAT405);
  }

  delete_request(req);

  return resp;
}


static int send_msg(struct sock_entry* se, char* buffer, int msg_len)
{
  if (buffer == NULL)
  {
    return -1;
  }

  int total = 0, bytes = 0;

  while (total < msg_len)
  {
    if ((bytes = send(se->sock, buffer + total, msg_len - total, 0)) == -1)
    {
      log_err("send_msg(): send(): %s", strerror(errno));
      remove_socket(se);
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


static void send_response(struct sock_entry* se, struct response* resp)
{
  if (resp == NULL)
  {
    // client disconnected
    return;
  }

  // log first line of resp header
  log_info("Response: %s", ks_list_get(resp->header, 0)->cp);

  // send each line of header
  int head_lines = ks_list_length(resp->header);
  for (int i = 0; i < head_lines; i++)
  {
    ks_datacont* line = ks_list_get(resp->header, i);
    if (send_msg(se, line->cp, line->size) == -1)
    {
      delete_response(resp);
      return;
    }
  }

  // end header
  send_msg(se, "\r\n", 2);

  // send resp body
  if (resp->content)
  {
    send_msg(se, resp->content, resp->content_length);
  }

  delete_response(resp);
}
