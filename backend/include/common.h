#ifndef _COMMON_H_
#define _COMMON_H_

#include <kylestructs.h>

#define HTTPVERSION "HTTP/1.1"

/* content types */
#define TEXTHTML "Content-Type: text/html\n"
#define TEXTCSS "Content-Type: text/css\n"
#define APPJS "Content-Type: application/javascript\n"
#define IMGICO "Content-Type: image/x-icon\n"

/* token struct definition */
#define UNAMELEN (16)
#define TOKENLEN (32)
#define TOKENLIFESPAN (7)

struct token_entry
{
  char token[TOKENLEN + 1];
  char user_id[32];
  char user_name[UNAMELEN + 1];
  int days;
  struct token_entry* next;
};

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
  const struct token_entry* client_info; // NULL token means client is not logged in
  char* content;
};

/* struct containing response info */
struct response
{
  /* list of strings */
  list* header;
  int content_length;
  char* content;
};

#endif
