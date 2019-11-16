
#ifndef _SERVER_H_
#define _SERVER_H_

#include <inttypes.h>


/* server settings */

#define BUFFERLEN 1024

#define MAXBUFFERMUL 10

#define MAXURILEN 256


/* successful statuses */

#define STAT200 "200 OK\n\n"


/* client errors */

#define STAT404 "404 Not Found\n\n"

#define STAT413 "413 Payload Too Large\n\n"


/* server errors */

#define STAT500 "500 Internal Server Error\n\n"

#define STAT501 "501 Not Implemented\n\n"

#define STAT505 "505 HTTP Version Not Supported\n\n"
 

/* struct containing stripped down request info */

struct request
{
  int method;
  char* uri;
  char* content;
};


/* struct containing response info */

struct response
{
  char* status;
  char* content_type;
  size_t content_length;
  char* content;
};


/* commandline options container */

struct options
{
  uint16_t server_port;
  uint32_t server_ip;
  int max_clients;
  int max_queue;
};


/* Fills out an options structure with information from args
   passed via commandline. Uses default options if no args 
   passed. */
static void get_options(const int argc, const char** argv, struct options* opts);


/* Takes in a structure of execution options and listens for
   incoming connections. Never returns. */
static void serve(const struct options* opts);


/* Allocates a buffer of limited size on the heap and returns it to
   the serve() function for precessing. */
static char* receive_request(int client_fd, char* ip);


/* Determines the request method being used, passes it to one 
   of the corresponding http_* functions, and finally returns the
   resulting response back to serve(). */
static char* process_request(char* request);


/* Sends the response to the client in chunks. */
static void send_response(char* request, int client_fd);


#endif

