

#ifndef _COMMON_H_
#define _COMMON_H_


/* server settings */

#define HTTPVERSION "HTTP/1.1"

#define MAXURILEN 256


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
  /* user_id is unsigned int, but we need it to be able to hold
     anonymous user_id (-1) */
  long int user_id;
  enum request_method method;
  char* uri;
  char* token;
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

