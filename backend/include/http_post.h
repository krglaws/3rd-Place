#ifndef _HTTP_POST_H_
#define _HTTP_POST_H_

#include <common.h>

#define COOKIE_TEMPLATE "Set-Cookie: logintoken=%s;Expires=Wed, 1 Jan 2121 00:00:00 GMT\n"


/* processes post request, returns response object */
struct response* http_post(struct request* req);


/* builds redirect response object */
//static struct response* redirect(const char* uri);


/* attempts login, returns response object containing result */
static struct response* post_login(const char* uname, const char* passwd);


/* attempts signup, returns response object containing result */
static struct response* post_signup(const char* uname, const char* passwd);

#endif
