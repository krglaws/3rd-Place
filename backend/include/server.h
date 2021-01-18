#ifndef _SERVER_H_
#define _SERVER_H_

#include <arpa/inet.h>
#include <common.h>

/* Server settings */
#define DEFAULT_PORT (80)
#define DEFAULT_ADDR in6addr_loopback
#define DEFAULT_MAX_CLIENTS (100)


/* Constants related to incoming message sizes */
#define BUFFERLEN (1024)
#define MAXBUFFERMUL (10)
#define MAXURILEN (256)


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

static void delete_options(struct options* opts);


/* Fills out an options structure with information from args
   passed via commandline. Uses default options if no args 
   passed. */
static void get_options(const int argc, char* const* argv, struct options* opts);


/* Initializes services, signal handling, and sets up server socket */
static void init_server(struct options* opts);


/* Called when SIGTERM or SIGINT are received */
void terminate_server(const int signum);


/* Takes in a structure of execution options and listens for
   incoming connections. Never returns. */
static void serve();


/* Allocates a buffer of up to (MAXBUFFERMUL * BUFFERLEN) bytes
   on the heap and returns it to the serve() function for precessing. */
static char* receive_request(const int sock);


/* Parses request method into enum type */
static enum request_method get_request_method(const char* req_str);


/* Parses the request URI */
static char* get_uri(const enum request_method method, const char* request);


/* Gets login token from request */
static const struct auth_token* get_login_token(const char* req_str);


/* Gets request content from request string */
static char* get_request_content(const char* req_str);


/* Parses the request string into a 'request' struct, passes it to 
   the corresponding HTTP method handler, and returns a 'response' 
   struct. */
static struct response* process_request(const int sock, char* req_str);


/* Sends the contents of a buffer to a specified socket */
static int send_msg(const int sock, char* buffer, int msg_len);


/* Delete response object */
static void delete_response(struct response* resp);


/* Uses send_msg() to send the response in pieces */
static void send_response(const int sock, struct response* resp);


#endif
