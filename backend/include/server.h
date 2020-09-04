
#ifndef _SERVER_H_
#define _SERVER_H_


/* server settings */

#define BUFFERLEN (1024)

#define MAXBUFFERMUL (10)

#define MAXURILEN (256)


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
static char* receive_request(int client_fd, char* ip);


/* Parses request method into enum type */
static enum request_method get_request_method(char* req_str);


/* Gets login token from request */
static char* get_login_token(char* req_str);


/* Gets request content from request string */
static char* get_request_content(char* req_str);


/* Parses the request string into a 'request' struct, passes it to 
   the corresponding HTTP method handler, and returns a 'response' 
   struct. */
static struct response* process_request(datacont* req_str);


/* Sends the response to the client in chunks. */
static void send_response(const int client_fd, char* resp);


#endif

