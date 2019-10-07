
#ifndef _FALCON_H_
#define _FALCON_H_

struct options
{
  int port;
  int client;
};

struct options* valid_args(int argc, char** argv);

#endif

