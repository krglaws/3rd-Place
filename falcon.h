
#ifndef _FALCON_H_
#define _FALCON_H_

struct options
{
  unsigned short server_port;
  unsigned long server_ip;
  int max_clients;
  int max_queue;
};

void get_options(const int argc, const char** argv, struct options* opts);

void start_server(const struct options* opts);

#endif

