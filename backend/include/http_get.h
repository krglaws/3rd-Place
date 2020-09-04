
#ifndef _HTTP_GET_H_
#define _HTTP_GET_H_

struct response* http_get(struct request* req);

struct response* http_head(struct request* req);

char* parse_uri(char* uri_line);

char* load_file(char* path);

struct* response get_user(char* uname, char* token);

struct* response get_community(char* community, char* token);

struct* response get_post(char* post, char* token);

list** query_database_ls(char* query);

#endif

