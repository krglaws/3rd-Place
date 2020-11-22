
#ifndef _HTTP_GET_H_
#define _HTTP_GET_H_

struct response* http_get(struct request* req);

list* get_user_info(char* uname);

list** get_user_posts(char* uname);

list** get_user_comments(char* uname);

char* fill_in_posts(char* template, char* uname);

char* fill_in_comments(char* template, char* uname);

char* get_user(char* uname, char* token);

char* replace(char* template, char* this, char* withthat);

#endif

