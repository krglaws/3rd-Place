
#ifndef _HTTP_GET_H_
#define _HTTP_GET_H_

struct response* http_get(struct request* req);

struct response* http_head(struct request* req);

struct response* send_404();

char* parse_uri(char* uri_line);

char* load_file(char* path);

#endif

