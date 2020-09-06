
#ifndef _STAT_H_
#define _STAT_H_

#include <server.h>

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

#endif

