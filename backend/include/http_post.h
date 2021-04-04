#ifndef HTTP_POST_H
#define HTTP_POST_H

#include <server.h>
#include <response.h>

void init_http_post();

struct response* http_post(const struct request* req);

#endif
