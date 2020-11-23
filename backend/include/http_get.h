
#ifndef _HTTP_GET_H_
#define _HTTP_GET_H_

struct response* http_get(struct request* req);

char* fill_user_info(char* template, char* uname);

char* fill_user_posts(char* template, char* uname);

char* fill_user_comments(char* template, char* uname);

char* get_user(char* uname, char* token);

char* fill_post_info(char* template, char* postid);

char* fill_post_comments(char* template, char* postid);

char* get_post(char* postid, char* token);

char* replace(char* template, char* this, char* withthat);

#endif

