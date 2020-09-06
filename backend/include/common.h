
#ifndef _COMMON_H_
#define _COMMON_H_

#include <kylestructs.h>

#define HTTPVERSION "HTTP/1.1"

/* content types */

#define TEXTHTML "Content-Type: text/html\n"
#define TEXTCSS "Content-Type: text/css\n"
#define APPJS "Content-Type: application/javascript\n"
#define IMGICO "Content-Type: image/x-icon\n"

/* success reponses */
#define STAT200 (HTTPVERSION " 200 OK\n")

/* client errors */
#define STAT400 (HTTPVERSION " 400 Bad Request\n")
#define STAT404 (HTTPVERSION " 404 Not Found\n")
#define STAT413 (HTTPVERSION " 413 Payload Too Large\n")
#define STAT414 (HTTPVERSION " 414 URI Too Long\n")

/* server errors */
#define STAT500 (HTTPVERSION " 500 Internal Server Error\n")
#define STAT501 (HTTPVERSION " 501 Not Implemented\n")
#define STAT505 (HTTPVERSION " 505 HTTP Version Not Supported\n")

#define UNAMELEN (16)

/* request methods */
enum request_method
{
  GET_REQ,
  HEAD_REQ,
  PUT_REQ,
  POST_REQ,
  DELETE_REQ,
  BAD_REQ
};

/* struct containing stripped down request info */
struct request
{
  enum request_method method;
  char* uri;
  char* token; // NULL token means client is not logged in
  char* content;
};

/* struct containing response info */
struct response
{
  /* list of strings */
  list* header;
  char* content;
};

#endif

