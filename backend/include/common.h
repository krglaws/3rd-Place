

#ifndef _COMMON_H_
#define _COMMON_H_


/* server settings */

#define MAXURILEN 256


/* successful statuses */

#define STAT200 "200 OK\n\n"


/* client errors */

#define STAT400 "400 Bad Request\n\n"

#define STAT404 "404 Not Found\n\n"

#define STAT413 "413 Payload Too Large\n\n"


/* server errors */

#define STAT500 "500 Internal Server Error\n\n"

#define STAT501 "501 Not Implemented\n\n"

#define STAT505 "505 HTTP Version Not Supported\n\n"
 

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
  list* header;
  char* content;
};


/* struct containing response info */
struct response
{
  list* header;
  char* content;
};


#endif

