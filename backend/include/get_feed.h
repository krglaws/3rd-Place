#ifndef GET_FEED_H
#define GET_FEED_H

#include <server.h>
#include <response.h>

struct response* get_home(const struct request* req);

struct response* get_all(const struct request* req);

struct response* get_communities(const struct request* req);

#endif
