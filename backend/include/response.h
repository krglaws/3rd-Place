#ifndef RESPONSE_H
#define RESPONSE_H

/* import HTTPVERSION macro */
#include <server.h>
#include <kylestructs.h>

/* success reponses */
#define STAT200 HTTPVERSION " 200 OK\r\n"

/* redirect responses */
#define STAT302 HTTPVERSION " 302 Found\r\n"

/* client errors */
#define STAT400 HTTPVERSION " 400 Bad Request\r\n"
#define STAT403 HTTPVERSION " 403 Permission Denied\r\n"
#define STAT404 HTTPVERSION " 404 Not Found\r\n"
#define STAT405 HTTPVERSION " 405 Method Not Allowed\r\n"
#define STAT413 HTTPVERSION " 413 Payload Too Large\r\n"
#define STAT414 HTTPVERSION " 414 URI Too Long\r\n"

/* server errors */
#define STAT500 HTTPVERSION " 500 Internal Server Error\r\n"
#define STAT501 HTTPVERSION " 501 Not Implemented\r\n"
#define STAT505 HTTPVERSION " 505 HTTP Version Not Supported\r\n"

/* struct containing response info */
struct response
{
  /* list of header lines */
  ks_list* header;

  /* content might contain NULL bytes, so 
     can't rely 'content' being properly
     NULL terminated */
  int content_length;

  /* response content body */
  char* content;
};


/* builds error response */
struct response* response_error(const char* errstr);

/* builds redirect response */
struct response* response_redirect(const char* uri);

/* builds 200 ok response */
struct response* response_ok(const char* content);

#endif
