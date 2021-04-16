#ifndef POST_DELETE_H
#define POST_DELETE_H

#include <server.h>
#include <response.h>

struct response* delete_user(const struct request* req);

struct response* delete_post(const struct request* req);

struct response* delete_comment(const struct request* req);

struct response* delete_community(const struct request* req);

struct response* delete_moderator(const struct request* req);

#endif
