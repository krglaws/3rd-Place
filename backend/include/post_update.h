#ifndef POST_UPDATE_H
#define POST_UPDATE_H

#include <server.h>
#include <response.h>

struct response* update_user_about(const struct request* req);

struct response* update_user_password(const struct request* req);

struct response* update_post(const struct request* req);

struct response* update_comment(const struct request* req);

struct response* update_community_about(const struct request* req);

#endif
