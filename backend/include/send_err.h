
#ifndef _SEND_ERR_H_
#define _SEND_ERR_H_

/* client errors */
#define STAT400 (HTTPVERSION " 400 Bad Request\n")
#define STAT404 (HTTPVERSION " 404 Not Found\n")
#define STAT413 (HTTPVERSION " 413 Payload Too Large\n")
#define STAT414 (HTTPVERSION " 414 URI Too Long\n")

/* server errors */
#define STAT500 (HTTPVERSION " 500 Internal Server Error\n")
#define STAT501 (HTTPVERSION " 501 Not Implemented\n")
#define STAT505 (HTTPVERSION " 505 HTTP Version Not Supported\n")

struct response* send_err(char* status);

#endif

