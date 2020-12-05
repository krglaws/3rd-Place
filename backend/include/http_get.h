#ifndef _HTTP_GET_H_
#define _HTTP_GET_H_

/* html template paths */
#define HTML_MAIN "templates/main/main.html"
#define HTML_VOTE_WRAPPER "templates/main/vote_wrapper.html"
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

struct response* http_get(struct request* req);

char* replace(char* template, const char* this, const char* withthat);

char* fill_nav_login(char* template, const struct token_entry* client_info);

char* fill_user_info(char* template, char* uname);

char* load_user_post_stub_template();

char* fill_user_post_stub(char* template, const list* post_info, const struct token_entry* client_info);

char* fill_user_posts(char* template, char* uname, const struct token_entry* client_info);

char* fill_user_comments(char* template, char* uname, const struct token_entry* client_info);

char* get_user(char* uname, const struct token_entry* client_info);

char* fill_post_info(char* template, char* postid);

char* fill_post_comments(char* template, char* postid);

char* get_post(char* postid, const struct token_entry* client_info);

char* fill_community_info(char* template, char* community_name);

char* fill_community_posts(char* template, char* community_name);

char* get_community(char* community_name, const struct token_entry* client_info);

#endif
