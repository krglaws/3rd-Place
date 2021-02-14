#ifndef SQL_MANAGER_H
#define SQL_MANAGER_H


/* user sql field names/indeces */
#define USERS_NUM_FIELDS            (8)
#define FIELD_USER_ID               "USER_ID"
#define FIELD_USER_NAME             "USER_NAME"
#define FIELD_USER_PASSWORD_HASH    "USER_PASSWORD_HASH"
#define FIELD_USER_ABOUT            "USER_ABOUT"
#define FIELD_USER_POINTS           "USER_POINTS"
#define FIELD_USER_POSTS            "USER_POST_COUNT"
#define FIELD_USER_COMMENTS         "USER_COMMENT_COUNT"
#define FIELD_USER_DATE_JOINED      "USER_DATE_JOINED"


/* post sql field names */
#define POSTS_NUM_FIELDS             (9)
#define FIELD_POST_ID                "POST_ID"
#define FIELD_POST_COMMUNITY_ID      "POST_COMMUNITY_ID"
#define FIELD_POST_COMMUNITY_NAME    "POST_COMMUNITY_NAME"
#define FIELD_POST_AUTHOR_ID         "POST_AUTHOR_ID"
#define FIELD_POST_AUTHOR_NAME       "POST_AUTHOR_NAME"
#define FIELD_POST_TITLE             "POST_TITLE"
#define FIELD_POST_BODY              "POST_BODY"
#define FIELD_POST_POINTS            "POST_POINTS"
#define FIELD_POST_DATE_POSTED       "POST_DATE_POSTED"


/* comment sql field names */
#define COMMENTS_NUM_FIELDS               (9)
#define FIELD_COMMENT_ID                  "COMMENT_ID"
#define FIELD_COMMENT_POST_ID             "COMMENT_POST_ID"
#define FIELD_COMMENT_POST_TITLE          "COMMENT_POST_TITLE"
#define FIELD_COMMENT_COMMUNITY_NAME      "COMMENT_COMMUNITY_NAME"
#define FIELD_COMMENT_AUTHOR_ID           "COMMENT_AUTHOR_ID"
#define FIELD_COMMENT_AUTHOR_NAME         "COMMENT_AUTHOR_NAME"
#define FIELD_COMMENT_BODY                "COMMENT_BODY"
#define FIELD_COMMENT_POINTS              "COMMENT_POINTS"
#define FIELD_COMMENT_DATE_POSTED         "COMMENT_DATE_POSTED"


/* community sql field names */
#define COMMUNITIES_NUM_FIELDS          (6)
#define FIELD_COMMUNITY_ID              "COMMUNITY_ID"
#define FIELD_COMMUNITY_OWNER_ID        "COMMUNITY_OWNER_ID"
#define FIELD_COMMUNITY_NAME            "COMMUNITY_NAME"
#define FIELD_COMMUNITY_ABOUT           "COMMUNITY_ABOUT"
#define FIELD_COMMUNITY_MEMBERS         "COMMUNITY_MEMBERS"
#define FIELD_COMMUNITY_DATE_CREATED    "COMMUNITY_DATE_CREATED"


/* moderator sql field names */
#define MODERATORS_NUM_FIELDS          (3)
#define FIELD_MODERATOR_ID             "MODERATOR_ID"
#define FIELD_MODERATOR_USER_ID        "MODERATOR_USER_ID"
#define FIELD_MODERATOR_COMMUNITY_ID   "MODERATOR_COMMUNITY_ID"


/* administrator sql fields */
#define ADMINISTRATORS_NUM_FIELDS      (1)
#define FIELD_ADMINISTRATOR_USER_ID    "ADMINISTRATOR_USER_ID"


/* subscription field names */
#define SUBSCRIPTIONS_NUM_FIELDS     (5)
#define FIELD_SUB_ID                 "SUB_ID"
#define FIELD_SUB_USER_ID            "SUB_USER_ID"
#define FIELD_SUB_USER_NAME          "SUB_USER_NAME"
#define FIELD_SUB_COMMUNITY_ID       "SUB_COMMUNITY_ID"
#define FIELD_SUB_COMMUNITY_NAME     "SUB_COMMUNITY_NAME"


/* post up vote field names */
#define POST_UPVOTES_NUM_FIELDS     (3)
#define FIELD_POST_UPVOTE_ID        "POST_UP_VOTE_ID"
#define FIELD_POST_UPVOTE_POST_ID   "POST_UP_VOTE_POST_ID"
#define FIELD_POST_UPVOTE_USER_ID   "POST_UP_VOTE_USER_ID"


/* post down vote field names */
#define POST_DOWNVOTES_NUM_FIELDS     (3)
#define FIELD_POST_DOWNVOTE_ID        "POST_DOWN_VOTE_ID"
#define FIELD_POST_DOWNVOTE_POST_ID   "POST_DOWN_VOTE_POST_ID"
#define FIELD_POST_DOWNVOTE_USER_ID   "POST_DOWN_VOTE_USER_ID"


/* comment up vote field names */
#define COMMENT_UPVOTES_NUM_FIELDS     (3)
#define FIELD_COMMENT_UPVOTE_ID        "COMMENT_UP_VOTE_ID"
#define FIELD_COMMENT_UPVOTE_POST_ID   "COMMENT_UP_VOTE_POST_ID"
#define FIELD_COMMENT_UPVOTE_USER_ID   "COMMENT_UP_VOTE_USER_ID"


/* comment down vote field names */
#define COMMENT_DOWNVOTES_NUM_FIELDS     (3)
#define FIELD_COMMENT_DOWNVOTE_ID        "COMMENT_DOWN_VOTE_ID"
#define FIELD_COMMENT_DOWNVOTE_POST_ID   "COMMENT_DOWN_VOTE_POST_ID"
#define FIELD_COMMENT_DOWNVOTE_USER_ID   "COMMENT_DOWN_VOTE_USER_ID"


/* query templates */
#define QUERY_USERS_BY_NAME "SELECT * FROM users WHERE name = '%s';"

#define QUERY_ALL_POSTS "SELECT* FROM posts;"
#define QUERY_POSTS_BY_ID "SELECT * FROM posts WHERE id = %s;"
#define QUERY_POSTS_BY_AUTHOR_NAME "SELECT * FROM posts WHERE author_name = '%s';"
#define QUERY_POSTS_BY_COMMUNITY_NAME "SELECT * FROM posts WHERE community_name = '%s';"

#define QUERY_COMMENTS_BY_ID "SELECT * FROM comments WHERE comment_id = %s;"
#define QUERY_COMMENTS_BY_AUTHOR_NAME "SELECT * FROM comments WHERE author_name = '%s';"
#define QUERY_COMMENTS_BY_POST_ID "SELECT * FROM comments WHERE post_id = %s;"

#define QUERY_ALL_COMMUNITIES "SELECT * FROM communities;"
#define QUERY_COMMUNITIES_BY_NAME "SELECT * FROM communities WHERE name = '%s';"

#define QUERY_MODERATORS_BY_COMMUNITY_ID_USER_ID "SELECT * FROM moderators WHERE community_id = %s AND user_id = %s;"

#define QUERY_ADMINISTRATORS_BY_USER_ID "SELECT * FROM administrators WHERE user_id = %s;"

#define QUERY_SUBSCRIPTIONS_BY_USER_ID "SELECT * FROM subscriptions WHERE user_id = %s;"

#define QUERY_POST_UPVOTES_BY_POST_ID_USER_ID "SELECT * FROM post_up_votes WHERE post_id = %s AND user_id = %s;"
#define QUERY_POST_DOWNVOTES_BY_POST_ID_USER_ID "SELECT * FROM post_down_votes WHERE post_id = %s AND user_id = %s;"

#define QUERY_COMMENT_UPVOTES_BY_COMMENT_ID_USER_ID "SELECT * FROM comment_up_votes WHERE comment_id = %s AND user_id = %s;"
#define QUERY_COMMENT_DOWNVOTES_BY_COMMENT_ID_USER_ID "SELECT * FROM comment_down_votes WHERE comment_id = %s AND user_id = %s;"
#define QUERY_SUBSCRIPTIONS_BY_COMMUNITY_ID_USER_ID "SELECT * FROM subscriptions WHERE community_id = %s AND user_id = %s;"

#define INSERT_NEW_USER "INSERT INTO users (name, password_hash, date_joined) VALUES ('%s', '%s', UNIX_TIMESTAMP());"
#define INSERT_POST_UPVOTE "INSERT INTO post_up_votes (user_id, post_id) VALUES (%s, %s);"
#define INSERT_POST_DOWNVOTE "INSERT INTO post_down_votes (user_id, post_id) VALUES (%s, %s);"
#define INSERT_COMMENT_UPVOTE "INSERT INTO comment_up_votes (user_id, post_id) VALUES (%s, %s);"
#define INSERT_COMMENT_DOWNVOTE "INSERT INTO comment_up_votes (user_id, post_id) VALUES (%s, %s);"

#define TOGGLE_POST_UPVOTE "CALL TogglePostUpvote(%s, %s);"
#define TOGGLE_POST_DOWNVOTE "CALL TogglePostDownvote(%s, %s);"
#define TOGGLE_COMMENT_UPVOTE "CALL ToggleCommentUpvote(%s, %s);"
#define TOGGLE_COMMENT_DOWNVOTE "CALL ToggleCommentDOwnvote(%s, %s);"

#include <kylestructs.h>

void init_sql_manager();

void terminate_sql_manager();


/* The following functions return lists of hashmaps.
 * Each index in the list is a hashmap which represents
 * a table row. Each map maps the field name (a string)
 * to the field value.
 */

ks_list* query_users_by_name(const char* user_name);


ks_list* query_all_posts();

ks_list* query_posts_by_id(const char* id);

ks_list* query_posts_by_author_name(const char* user_name);

ks_list* query_posts_by_community_name(const char* community_name);


ks_list* query_all_communities();

ks_list* query_communities_by_name(const char* community_name);


ks_list* query_moderators_by_community_id_user_id(const char* community_id, const char* user_id);


ks_list* query_administrators_by_user_id(const char* user_id);


ks_list* query_comments_by_id(const char* comment_id);

ks_list* query_comments_by_author_name(const char* author_name);

ks_list* query_comments_by_post_id(const char* post_id);


ks_list* query_subscriptions_by_user_id(const char* user_id);


ks_list* query_post_upvotes_by_post_id_user_id(const char* post_id, const char* user_id);

ks_list* query_post_downvotes_by_post_id_user_id(const char* post_id, const char* user_id);

ks_list* query_comment_upvotes_by_post_id_user_id(const char* comment_id, const char* user_id);

ks_list* query_comment_downvotes_by_post_id_user_id(const char* comment_id, const char* user_id);


ks_list* query_subscriptions_by_community_id_user_id(const char* community_id, const char* id);


/* Insert queries return -1 if the insert statement failed
 */

int insert_new_user(const char* user_name, const char* passwd_hash);

int toggle_post_upvote(const char* post_id, const char* user_id);

int toggle_post_downvote(const char* post_id, const char* user_id);

int toggle_comment_upvote(const char* comment_id, const char* user_id);

int toggle_comment_downvote(const char* comment_id, const char* user_id);

#endif
