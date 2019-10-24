
#ifndef _SERVER_H_
#define _SERVER_H_

#define BUFFERLEN 1024
#define MAXBUFFERMUL 10

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

