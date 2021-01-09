#ifndef _HTTP_POST_H_
#define _HTTP_POST_H_

#include <common.h>

#define REDIRECT_TEMPLATE "Location: %s\n"
#define COOKIE_TEMPLATE "Set-Cookie: logintoken=%s;Expires=Wed, 1 Jan 2121 00:00:00 GMT\n"


/* processes post request, returns response object */
struct response* http_post(struct request* req);


/* used to retrieve string arguments from 
    argument map */
static const char* get_arg(const ks_treemap* args, const char* argname);


/* add key-value pair to argument map. used by parse_args() */
static int add_key_val(ks_treemap* map, char* token);


/* parses the argument list in post content body */
static ks_treemap* parse_args(char* str);


/* builds redirect response object */
static struct response* redirect(const char* uri);


/* attempts login, returns response object containing result */
static struct response* post_login(const char* uname, const char* passwd);


/* attempts signup, returns response object containing result */
static struct response* post_signup(const char* uname, const char* passwd);

#endif
