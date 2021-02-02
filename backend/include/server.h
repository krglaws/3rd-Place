#ifndef _SERVER_H_
#define _SERVER_H_

#include <arpa/inet.h>
#include <common.h>

/* Server settings */
#define DEFAULT_PORT (80)
#define DEFAULT_ADDR in6addr_loopback
#define DEFAULT_MAX_CLIENTS (100)


/* Constants related to incoming message sizes */
#define MAXREQUESTSIZE (10240)


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
