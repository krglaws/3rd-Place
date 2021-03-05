#ifndef HTTP_POST_H
#define HTTP_POST_H

#include <kylestructs.h>

#include <server.h>
#include <auth_manager.h>
#include <response.h>

/* used for successful login/signup responses */
#define COOKIE_TEMPLATE "Set-Cookie: logintoken=%s;Expires=Wed, 1 Jan 2121 00:00:00 GMT\r\n"

/* processes post request, returns response object */
struct response* http_post(struct request* req);

#endif
