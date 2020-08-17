
#ifndef _SERVER_H_
#define _SERVER_H_


/* server settings */

#define BUFFERLEN 1024

#define MAXBUFFERMUL 10


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


/* Allocates a buffer of up to (MAXBUFFERMUL * BUFFERLEN) bytes
   on the heap and returns it to the serve() function for precessing. */
static datacont* receive_request(int client_fd, char* ip);


/* Parses the request string into a 'request' struct, passes it to 
   the corresponding HTTP method handler, and returns a 'response' 
   struct. */
static struct response* process_request(datacont* req_str);


/* Sends the response to the client in chunks. */
static void send_response(struct response* resp, const int client_fd);


#endif

