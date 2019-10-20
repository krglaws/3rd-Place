
#ifndef _SERVER_H_
#define _SERVER_H_


/* Fills out an options structure with information from args
   passed via commandline. Uses default options if no args 
   passed */
void get_options(int argc, char** argv, struct options* opts);


/* Takes in a structure of execution options and listens for
   incoming connections. Never returns. */
void start_server(struct options* opts);


struct request* parse_request(char* buffer,


#endif
