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
#define COMMENTS_NUM_FIELDS               (10)
#define FIELD_COMMENT_ID                  "COMMENT_ID"
#define FIELD_COMMENT_POST_ID             "COMMENT_POST_ID"
#define FIELD_COMMENT_POST_TITLE          "COMMENT_POST_TITLE"
#define FIELD_COMMENT_COMMUNITY_ID        "COMMENT_COMMUNITY_ID"
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
#define FIELD_SUB_COMMUNITY_ID       "SUB_COMMUNITY_ID"


/* post up vote field names */
#define POST_UP_VOTES_NUM_FIELDS     (3)
#define FIELD_POST_UP_VOTE_ID        "POST_UP_VOTE_ID"
#define FIELD_POST_UP_VOTE_POST_ID   "POST_UP_VOTE_POST_ID"
#define FIELD_POST_UP_VOTE_USER_ID   "POST_UP_VOTE_USER_ID"


/* post down vote field names */
#define POST_DOWN_VOTES_NUM_FIELDS     (3)
#define FIELD_POST_DOWN_VOTE_ID        "POST_DOWN_VOTE_ID"
#define FIELD_POST_DOWN_VOTE_POST_ID   "POST_DOWN_VOTE_POST_ID"
#define FIELD_POST_DOWN_VOTE_USER_ID   "POST_DOWN_VOTE_USER_ID"


/* comment up vote field names */
#define COMMENT_UP_VOTES_NUM_FIELDS     (3)
#define FIELD_COMMENT_UP_VOTE_ID        "COMMENT_UP_VOTE_ID"
#define FIELD_COMMENT_UP_VOTE_POST_ID   "COMMENT_UP_VOTE_POST_ID"
#define FIELD_COMMENT_UP_VOTE_USER_ID   "COMMENT_UP_VOTE_USER_ID"


/* comment down vote field names */
#define COMMENT_DOWN_VOTES_NUM_FIELDS     (3)
#define FIELD_COMMENT_DOWN_VOTE_ID        "COMMENT_DOWN_VOTE_ID"
#define FIELD_COMMENT_DOWN_VOTE_POST_ID   "COMMENT_DOWN_VOTE_POST_ID"
#define FIELD_COMMENT_DOWN_VOTE_USER_ID   "COMMENT_DOWN_VOTE_USER_ID"

#include <kylestructs.h>

void init_sql_manager();

void terminate_sql_manager();

/* The following functions return lists of hashmaps.
 * Each index in the list is a hashmap which represents
 * a table row. Each maps the field name (a string)
 * to a field value (also a string).
 */

ks_list* query_users_by_name(const char* user_name);

ks_list* query_all_posts();

ks_list* query_posts_by_id(const char* id);

ks_list* query_posts_by_author_name(const char* user_name);

ks_list* query_posts_by_community_id(const char* community_id);

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

/* Return 0 on success, -1 on failure */
int sql_toggle_subscribe(const char* community_id, const char* user_id);

int sql_toggle_post_up_vote(const char* post_id, const char* user_id);

int sql_toggle_post_down_vote(const char* post_id, const char* user_id);

int sql_toggle_comment_up_vote(const char* comment_id, const char* user_id);

int sql_toggle_comment_down_vote(const char* comment_id, const char* user_id);


/* Return heap-allocated string ID of the newly created item (free() after use), NULL on failure */
char* sql_create_user(const char* user_name, const char* passwd_hash, const char* about);

char* sql_create_comment(const char* user_id, const char* post_id, const char* community_id, const char* body);

char* sql_create_post(const char* user_id, const char* community_id, const char* title, const char* body);

char* sql_create_community(const char* user_id, const char* community_name, const char* about);


/* Return 0 on success, -1 on failure */
int sql_delete_user(const char* user_id);

int sql_delete_comment(const char* comment_id);

int sql_delete_post(const char* post_id);

int sql_delete_community(const char* community_id);

int sql_delete_moderator(const char* community_id, const char* user_id);

int sql_delete_administrator(const char* user_id);

#endif
