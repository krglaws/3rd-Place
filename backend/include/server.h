#ifndef SERVER_H
#define SERVER_H

#include <kylestructs.h>

/* struct in6_addr definition */
#include <arpa/inet.h>

/* Server settings */
#define DEFAULT_PORT (80)
#define DEFAULT_ADDR in6addr_loopback
#define DEFAULT_MAX_CLIENTS (100)

#define HTTPVERSION "HTTP/1.1"
#define HTTPVERSION_ALT "HTTP/1.0"

#define MAXREQUESTSIZE (10240)

/* struct response definition */
#include <response.h>

/* struct auth_token definition */
#include <auth_manager.h>

/* Prints usage string */
void usage(const char* name);


/* Commandline options container */
struct options
{
  struct in6_addr server_addr;
  uint16_t server_port;
  int max_clients;
  char* logpath;
};


/* Fills out an options structure with information from args
   passed via commandline. Uses default options if no args 
   passed. */
static void get_options(const int argc, char* const* argv, struct options* opts);

static void delete_options(struct options* opts);


/* Initializes services, signal handling, and sets up server socket */
static void init_server(struct options* opts);


/* Called when SIGTERM or SIGINT are received */
void terminate_server(const int signum);


/* Takes in a structure of execution options and listens for
   incoming connections. Never returns. */
static void serve();


/* struct containing stripped down request info */
struct request
{
  /* if client is logged in, this will
     contain the clients user name and
     user ID */
  struct auth_token* client_info;

  /* will accept either 1.1 or 1.0 */
  char* http_version;

  /* POST, GET, PUT, DELETE, or HEAD */
  char* method;

  /* path to requested resource */
  char* uri;

  /* if a query string is present,
     this will contain the key-value
     pairs */
  ks_hashmap* query;

  /* key-value pair for each header */
  ks_hashmap* header;

  /* cookies parsed from header */
  ks_hashmap* cookies;

  /* key-value pair for each parameter
     in request content */
  ks_hashmap* content;  
};


/* Parses the raw request data into a request struct */
static struct request* parse_request(char* req_buf);

static void delete_request(struct request* req);


/* Parses the request string into a 'request' struct, passes it to 
   the corresponding HTTP method handler, and returns a 'response' 
   struct. */
static struct response* process_request(const int sock);


/* Sends the contents of a buffer to a specified socket */
static int send_msg(const int sock, char* buffer, int msg_len);


/* Delete response object */
static void delete_response(struct response* resp);


/* Uses send_msg() to send the response in pieces */
static void send_response(const int sock, struct response* resp);


#endif
