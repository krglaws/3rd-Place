#ifndef _HTTP_POST_H_
#define _HTTP_POST_H_

#include <common.h>

#define COOKIE_TEMPLATE "Set-Cookie: logintoken=%s;Expires=Wed, 1 Jan 2121 00:00:00 GMT\r\n"


/* processes post request, returns response object */
struct response* http_post(struct request* req);


/* builds redirect response object */
//static struct response* redirect(const char* uri);


/* attempts login, returns response object containing result */
static struct response* post_login(const char* uname, const char* passwd);


/* attempts signup, returns response object containing result */
static struct response* post_signup(const char* uname, const char* passwd);


/* toggles a users vote on post or comment */
static struct response* post_vote(const char* type, const char* direction, const char* id, const struct auth_token* client_info);


#endif
