#ifndef _SENDERR_H_
#define _SENDERR_H_

// need HTTPVERSION macro
#include <common.h>


/* success reponses */
#define STAT200 HTTPVERSION " 200 OK\n"

/* redirect responses */
#define STAT302 HTTPVERSION " 302 Found\n"

/* client errors */
#define STAT400 HTTPVERSION " 400 Bad Request\n"
#define STAT404 HTTPVERSION " 404 Not Found\n"
#define STAT413 HTTPVERSION " 413 Payload Too Large\n"
#define STAT414 HTTPVERSION " 414 URI Too Long\n"

/* server errors */
#define STAT500 HTTPVERSION " 500 Internal Server Error\n"
#define STAT501 HTTPVERSION " 501 Not Implemented\n"
#define STAT505 HTTPVERSION " 505 HTTP Version Not Supported\n"


enum http_errno {
  ERR_BAD_REQ,
  ERR_NOT_FOUND,
  ERR_MSG_TOO_BIG,
  ERR_URI_TOO_LONG,
  ERR_INTERNAL,
  ERR_NOT_IMPL,
  ERR_HTTP_VERS_NOT_SUPP
};

/* sends error response based on eno arg */
struct response* senderr(enum http_errno eno);

/* sends a redirect to /login */
//struct response* login_redirect();

struct response* redirect(const char* uri);

#endif
