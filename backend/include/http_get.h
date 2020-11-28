#ifndef _HTTP_GET_H_
#define _HTTP_GET_H_

/* html template paths */
#define HTML_MAIN "templates/main/main.html"
#define HTML_USER "templates/user/user.html"
#define HTML_USER_POST_STUB "templates/user/post_stub.html"
#define HTML_USER_COMMENT_STUB "templates/user/comment_stub.html"
#define HTML_POST "templates/post/post.html"
#define HTML_POST_COMMENT "templates/post/comment.html"
#define HTML_COMMUNITY "templates/community/community.html"
#define HTML_COMMUNITY_POST_STUB "templates/community/post_stub.html"

/* paths hereafter need the leading '/' since
   they will be requested by the client browser
   directly */

/* css paths */
#define CSS_MAIN "/templates/main/main.css"
#define CSS_USER "/templates/user/user.css"
#define CSS_POST "/templates/post/post.css"
#define CSS_COMMUNITY "/templates/community/community.css"
#define CSS_FEED "/templates/feed/feed.css"

/* js paths */
#define JS_USER "/templates/user/user.js"
#define JS_COMMUNITY "/templates/community/community.js"

/* query templates */
#define QUERY_USER_BY_UNAME "SELECT * FROM users WHERE name = '%s';"
#define QUERY_POSTS_BY_UNAME "SELECT * FROM posts WHERE author_name = '%s';"
#define QUERY_COMMENTS_BY_UNAME "SELECT * FROM comments WHERE author_name = '%s';"
#define QUERY_POST_BY_UUID "SELECT * FROM posts WHERE id = '%s';"
#define QUERY_POSTS_BY_COMMUNITY_NAME "SELECT * FROM posts WHERE community_name = '%s';"
#define QUERY_COMMENTS_BY_POSTID "SELECT * FROM comments WHERE post_id = '%s';"
#define QUERY_COMMUNITY_BY_NAME "SELECT * FROM communities WHERE name = '%s';"

struct response* http_get(struct request* req);

char* replace(char* template, const char* this, const char* withthat);

char* fill_user_info(char* template, char* uname);

char* fill_user_posts(char* template, char* uname);

char* fill_user_comments(char* template, char* uname);

char* get_user(char* uname, char* token);

char* fill_post_info(char* template, char* postid);

char* fill_post_comments(char* template, char* postid);

char* get_post(char* postid, char* token);

char* fill_community_info(char* template, char* community_name);

char* fill_community_posts(char* template, char* community_name);

char* get_community(char* community_name, char* token);

#endif
