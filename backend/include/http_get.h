
#ifndef _HTTP_GET_H_
#define _HTTP_GET_H_

char* http_get(char* request);

char* http_head(char* request);

char* parse_uri(char* request);

char* load_file(char* uri);

#endif

