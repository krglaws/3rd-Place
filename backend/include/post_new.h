#ifndef POST_NEW_H
#define POST_NEW_H

#include <response.h>
#include <auth_manager.h>

struct response* login(const struct request* req);

struct response* logout(const struct request* req);

struct response* signup(const struct request* req);

struct response* vote(const struct request* req);

struct response* subscribe(const struct request* req);

struct response* new_comment(const struct request* req);

struct response* new_post(const struct request* req);

struct response* new_community(const struct request* req);

#endif
