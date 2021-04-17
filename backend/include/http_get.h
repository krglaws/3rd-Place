#ifndef HTTP_GET_H
#define HTTP_GET_H

#include <stdbool.h>
#include <time.h>
#include <kylestructs.h>

/* struct request definition */
#include <server.h>

/* struct auth_token definition */
#include <auth_manager.h>

/* html template paths */
#define HTML_MAIN "templates/main/main.html"
#define HTML_LOGIN "templates/forms/login.html"
#define HTML_ALREADY_LOGGED_IN "templates/forms/already_logged_in.html"
#define HTML_POST_VOTE_WRAPPER "templates/main/post_vote_wrapper.html"
#define HTML_COMMENT_VOTE_WRAPPER "templates/main/comment_vote_wrapper.html"
#define HTML_USER "templates/user/user.html"
#define HTML_USER_POST "templates/user/post.html"
#define HTML_USER_COMMENT "templates/user/comment.html"
#define HTML_POST "templates/post/post.html"
#define HTML_POST_COMMENT "templates/post/comment.html"
#define HTML_COMMUNITY "templates/community/community.html"
#define HTML_COMMUNITY_MOD_ENTRY "templates/community/mod_entry.html"
#define HTML_COMMUNITY_POST "templates/community/post.html"
#define HTML_FEED "templates/feed/feed.html"
#define HTML_FEED_POST "templates/feed/post.html"
#define HTML_FEED_COMMUNITY "templates/feed/community.html"
#define HTML_NEW_POST "templates/forms/new_post.html"
#define HTML_NEW_COMMENT "templates/forms/new_comment.html"
#define HTML_NEW_COMMUNITY "templates/forms/new_community.html"
#define HTML_EDIT_POST "templates/forms/edit_post.html"
#define HTML_EDIT_USER "templates/forms/edit_user.html"
#define HTML_EDIT_COMMENT "templates/forms/edit_comment.html"
#define HTML_EDIT_COMMUNITY "templates/forms/edit_community.html"

void init_http_get();

struct response* http_get(const struct request* req);

#define UPVOTE_CLICKED_STATE "upvote-clicked"
#define UPVOTE_NOTCLICKED_STATE "upvote-notclicked"
#define DOWNVOTE_CLICKED_STATE "downvote-clicked"
#define DOWNVOTE_NOTCLICKED_STATE "downvote-notclicked"

/* Vote checking */
enum vote_type {
  UPVOTE,
  DOWNVOTE,
  NOVOTE
};

/* Display posts/comments in correct order (newest at the top) */
enum item_type
{
  POST_ITEM,
  COMMENT_ITEM,
  COMMUNITY_ITEM
};

/* returns UPVOTE, DOWNVOTE, or NOVOTE for a given post or comment ID and user ID*/
enum vote_type check_for_vote(enum item_type it, const char* vote_id, const char* user_id);

bool check_for_sub(const char* community_id, const struct auth_token* client_info);

time_t get_item_date(const ks_hashmap* item, enum item_type it);

ks_list* sort_items(ks_list* items, enum item_type it);

ks_list* merge_items(ks_list* lsA, ks_list* lsB, enum item_type it);

/* wraps main page data hashmap with hashmap containing nav info */
ks_hashmap* wrap_page_data(const struct auth_token* client_info, const ks_hashmap* page_data);

#endif
