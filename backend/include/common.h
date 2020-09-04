
#ifndef _COMMON_H_
#define _COMMON_H_

#define HTTPVERSION "HTTP/1.1"

/* content types */

#define TEXTHTML "Content-Type: text/html\n"

#define TEXTCSS "Content-Type: text/css\n"

#define APPJS "Content-Type: application/javascript\n"

#define IMGICO "Content-Type: image/x-icon\n"

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

