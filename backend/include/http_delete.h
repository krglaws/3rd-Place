#ifndef HTTP_DELETE_H
#define HTTP_DELETE_H

/* request defined here */
#include <server.h>

/* response defined here */
#include <response.h>

struct response* http_delete(struct request* req);

#endif
