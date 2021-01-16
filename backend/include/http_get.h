#ifndef _HTTP_GET_H_
#define _HTTP_GET_H_

#include <common.h>

/* html template paths */
#define HTML_MAIN "templates/main/main.html"
#define HTML_LOGIN "templates/login/login.html"
#define HTML_ALREADY_LOGGED_IN "templates/login/already_logged_in.html"
#define HTML_POST_VOTE_WRAPPER "templates/main/post_vote_wrapper.html"
#define HTML_COMMENT_VOTE_WRAPPER "templates/main/comment_vote_wrapper.html"
#define HTML_USER "templates/user/user.html"
#define HTML_USER_POST "templates/user/post.html"
#define HTML_USER_COMMENT "templates/user/comment.html"
#define HTML_POST "templates/post/post.html"
#define HTML_POST_COMMENT "templates/post/comment.html"
#define HTML_COMMUNITY "templates/community/community.html"
#define HTML_COMMUNITY_POST "templates/community/post.html"
#define HTML_FEED "templates/feed/feed.html"
#define HTML_FEED_POST "templates/feed/post.html"

/* paths hereafter need the leading '/' since
   they will be requested by the client browser
   directly */

/* css paths */
#define CSS_MAIN "/templates/main/main.css"
#define CSS_LOGIN "/templates/login/login.css"
#define CSS_USER "/templates/user/user.css"
#define CSS_POST "/templates/post/post.css"
#define CSS_COMMUNITY "/templates/community/community.css"
#define CSS_FEED "/templates/feed/feed.css"

/* js paths */
#define JS_USER "/templates/user/user.js"
#define JS_COMMUNITY "/templates/community/community.js"

struct response* http_get(struct request* req);

void add_nav_info(ks_hashmap* page_data, const struct auth_token* client_info);

#define UPVOTE_CLICKED_STATE "upvote-clicked"
#define UPVOTE_NOTCLICKED_STATE "upvote-notclicked"
#define DOWNVOTE_CLICKED_STATE "downvote-clicked"
#define DOWNVOTE_NOTCLICKED_STATE "downvote-notclicked"

enum login_error {
  LOGINERR_NONE,
  LOGINERR_BAD_LOGIN,
  LOGINERR_EMPTY,
  LOGINERR_UNAME_TAKEN
};

#define BAD_LOGIN_MSG "<p>Bad Login</p>"
#define UNAME_TAKEN_MSG "<p>Username Already Exists</p>"
#define EMPTY_INPUT_MSG "<p>Empty username or password</p>"

char* get_login(const struct auth_token* client_info, enum login_error err);

enum vote_type {
  UPVOTE,
  DOWNVOTE,
  NOVOTE
};

enum vote_item_type {
  POST_VOTE,
  COMMENT_VOTE
};

enum vote_type check_for_vote(const enum vote_item_type item_type, const char* vote_id, const char* user_id);

enum get_err {
  NO_GET_ERR,
  REDIRECT, // redirect to /login
  INTERNAL // send 500 err
};

/* Called by the get_* files in the event of an internal
    server error to notify http_get to send a 500 instead
    404. */
void set_error(enum get_err err);


/* Retrieves value of static int internal_error and sets it to 0 */
static enum get_err get_error();

#endif
