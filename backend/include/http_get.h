
#ifndef _HTTP_GET_H_
#define _HTTP_GET_H_

/* html template paths */
#define HTMLMAIN "templates/main/main.html"
#define HTMLUSER "templates/user/user.html"
#define HTMLPOST "templates/post/post.html"
#define HTMLUSERPOSTSTUB "templates/user/post_stub.html"

/* paths hereafter need the leading '/' since
   they will be requested by the client browser
   directly */

/* css paths */
#define CSSMAIN "/templates/main/main.css"
#define CSSUSER "/templates/user/user.css"
#define CSSPOST "/templates/post/post.css"

/* js paths */
#define JSUSER "/templates/user/user.js"


/* query templates */
#define QUSER_BY_UNAME "SELECT * FROM users WHERE uname = '%s';"
#define QPOST_BY_UNAME "SELECT * FROM posts WHERE author = '%s';"


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

