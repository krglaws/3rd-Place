#ifndef _COMMON_H_
#define _COMMON_H_

#include <kylestructs.h>

#define HTTPVERSION "HTTP/1.1"

/* content types */
#define TEXTHTML "Content-Type: text/html\r\n"
#define TEXTCSS "Content-Type: text/css\r\n"
#define APPJS "Content-Type: application/javascript\r\n"
#define IMGICO "Content-Type: image/x-icon\r\n"

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
  const struct auth_token* client_info; // NULL token means client is not logged in
  char* content;
};

/* struct containing response info */
struct response
{
  /* list of strings */
  ks_list* header;
  int content_length;
  char* content;
};

#endif
