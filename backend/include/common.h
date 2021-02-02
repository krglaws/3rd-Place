#ifndef _COMMON_H_
#define _COMMON_H_

#include <kylestructs.h>

#define HTTPVERSION "HTTP/1.1"
#define HTTPVERSION_ALT "HTTP/1.0"

/* content types */
#define TEXTHTML "Content-Type: text/html\r\n"
#define TEXTCSS "Content-Type: text/css\r\n"
#define APPJS "Content-Type: application/javascript\r\n"
#define IMGICO "Content-Type: image/x-icon\r\n"

/* struct containing stripped down request info */
struct request
{
  /* if client is logged in, this will
     contain the clients user name and
     user ID */
  struct auth_token* client_info;

  /* will accept either 1.1 or 1.0 */
  char* http_version;

  /* POST, GET, PUT, DELETE, or HEAD */
  char* method;

  /* path to requested resource */
  char* uri;

  /* if a query string is present,
     this will contain the key-value
     pairs */
  ks_hashmap* query;

  /* key-value pair for each header */
  ks_hashmap* header;

  /* cookies parsed from header */
  ks_hashmap* cookies;

  /* key-value pair for each parameter
     in request content */
  ks_hashmap* content;  
};


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

#endif
