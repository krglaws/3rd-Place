
#ifndef _FALCON_H_
#define _FALCON_H_


struct options
{
  uint16_t server_port;
  uint32_t server_ip;
  int max_clients;
  int max_queue;
};


/* Fetches execution options from argv[] and fills out 'struct options' */
static void get_options(const int argc, const char** argv, struct options* opts);


/* Called by main(). Continuously loops, listening for incoming connections 
   until C^ received, or until everything crashes and burns */
static int start_server(const struct options* opts);


/* when C^ is pressed, the main thread signals the other threads to
   close all connections and exit */
static void handle_sigint(int signum);


#endif

