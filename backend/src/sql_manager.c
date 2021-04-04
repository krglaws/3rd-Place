#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <mysql.h>
#include <kylestructs.h>

#include <log_manager.h>
#include <load_file.h>
#include <string_map.h>
#include <sql_manager.h>


// one of these structs for each table in sql
// is used for retrieving data and storing for
// processing.
struct table_info
{
  const char** field_table;
  const int* field_lengths;
};

static int sql_exec(MYSQL_STMT* stmt, va_list ap);
static ks_list* sql_select(const struct table_info* table_info, MYSQL_STMT* stmt, ...);
static int sql_procedure(MYSQL_STMT* stmt, ...);
static char* sql_function(MYSQL_STMT* stmt, ...);
static void terminate_sql_manager();


/********************/
/* users table info */

static const char* users_field_table[USERS_NUM_FIELDS] =
{
  FIELD_USER_ID,
  FIELD_USER_NAME,
  FIELD_USER_PASSWORD_HASH,
  FIELD_USER_ABOUT,
  FIELD_USER_POINTS,
  FIELD_USER_POSTS,
  FIELD_USER_COMMENTS,
  FIELD_USER_DATE_JOINED
};

static const int users_field_lengths[USERS_NUM_FIELDS] =
{
  INT_BUF_LEN,
  USER_NAME_BUF_LEN,
  USER_PASSWD_BUF_LEN,
  USER_ABOUT_BUF_LEN,
  INT_BUF_LEN,
  INT_BUF_LEN,
  INT_BUF_LEN,
  INT_BUF_LEN
};
 
static const struct table_info users_table_info = 
{
  users_field_table,
  users_field_lengths
};

// user queries and their prepared statements
static MYSQL_STMT* stmt_query_user_by_id = NULL;
ks_hashmap* query_user_by_id(const char* user_id)
{
  ks_list* ls;
  if ((ls = sql_select(&users_table_info, stmt_query_user_by_id, user_id)) == NULL)
  {
    return NULL;
  }

  ks_datacont* dc = ks_list_get(ls, 0);
  ks_hashmap* row0 = dc->hm;
  dc->hm = NULL;
  ks_list_delete(ls);

  return row0;
}

static MYSQL_STMT* stmt_query_user_by_name = NULL;
ks_hashmap* query_user_by_name(const char* user_name)
{
  ks_list* ls;
  if ((ls = sql_select(&users_table_info, stmt_query_user_by_name, user_name)) == NULL)
  {
    return NULL;
  }

  ks_datacont* dc = ks_list_get(ls, 0);
  ks_hashmap* row0 = dc->hm;
  dc->hm = NULL;
  ks_list_delete(ls);

  return row0;
}

static MYSQL_STMT* stmt_create_user = NULL;
char* sql_create_user(const char* user_name, const char* password_hash, const char* about)
{
  return sql_function(stmt_create_user, user_name, password_hash, about);
}

static MYSQL_STMT* stmt_update_user_about = NULL;
int sql_update_user_about(const char* user_id, const char* about)
{
  return sql_procedure(stmt_update_user_about, user_id, about);
}

static MYSQL_STMT* stmt_update_user_password_hash = NULL;
int sql_update_user_password_hash(const char* user_id, const char* password_hash)
{
  return sql_procedure(stmt_update_user_password_hash, user_id, password_hash);
}

static MYSQL_STMT* stmt_delete_user = NULL;
int sql_delete_user(const char* user_id)
{
  return sql_procedure(stmt_delete_user, user_id);
}


/********************/
/* posts table info */

static const char* posts_field_table[POSTS_NUM_FIELDS] =
{
  FIELD_POST_ID,
  FIELD_POST_COMMUNITY_ID,
  FIELD_POST_COMMUNITY_NAME,
  FIELD_POST_AUTHOR_ID,
  FIELD_POST_AUTHOR_NAME,
  FIELD_POST_TITLE,
  FIELD_POST_BODY,
  FIELD_POST_POINTS,
  FIELD_POST_DATE_POSTED
};

static const int posts_field_lengths[POSTS_NUM_FIELDS] =
{
  INT_BUF_LEN,
  INT_BUF_LEN,
  COMMUNITY_NAME_BUF_LEN,
  INT_BUF_LEN,
  USER_NAME_BUF_LEN,
  POST_TITLE_BUF_LEN,
  POST_BODY_BUF_LEN,
  INT_BUF_LEN,
  INT_BUF_LEN
};

static const struct table_info posts_table_info =
{
  posts_field_table,
  posts_field_lengths
};

static MYSQL_STMT* stmt_query_post_by_id = NULL;
ks_hashmap* query_post_by_id(const char* id)
{
  ks_list* ls;
  if ((ls = sql_select(&posts_table_info, stmt_query_post_by_id, id)) == NULL)
  {
    return NULL;
  }

  ks_datacont* dc = ks_list_get(ls, 0);
  ks_hashmap* row0 = dc->hm;
  dc->hm = NULL;
  ks_list_delete(ls);

  return row0;
}

static MYSQL_STMT* stmt_query_all_posts = NULL;
ks_list* query_all_posts()
{
  return sql_select(&posts_table_info, stmt_query_all_posts);
}

static MYSQL_STMT* stmt_query_posts_by_author_name = NULL;
ks_list* query_posts_by_author_name(const char* author_name)
{
  return sql_select(&posts_table_info, stmt_query_posts_by_author_name, author_name);
}

static MYSQL_STMT* stmt_query_posts_by_community_id = NULL;
ks_list* query_posts_by_community_id(const char* community_id)
{
  return sql_select(&posts_table_info, stmt_query_posts_by_community_id, community_id);
}

static MYSQL_STMT* stmt_query_posts_by_community_name = NULL;
ks_list* query_posts_by_community_name(const char* community_name)
{
  return sql_select(&posts_table_info, stmt_query_posts_by_community_name, community_name);
}

static MYSQL_STMT* stmt_create_post = NULL;
char* sql_create_post(const char* user_id, const char* community_id, const char* title, const char* body)
{
  return sql_function(stmt_create_post, user_id, community_id, title, body);
}

static MYSQL_STMT* stmt_update_post_body = NULL;
int sql_update_post_body(const char* post_id, const char* body)
{
  return sql_procedure(stmt_update_post_body, post_id, body);
}

static MYSQL_STMT* stmt_delete_post = NULL;
int sql_delete_post(const char* post_id)
{
  return sql_procedure(stmt_delete_post, post_id);
}


/***********************/
/* comments table info */

static const char* comments_field_table[COMMENTS_NUM_FIELDS] =
{
  FIELD_COMMENT_ID,
  FIELD_COMMENT_POST_ID,
  FIELD_COMMENT_POST_TITLE,
  FIELD_COMMENT_COMMUNITY_ID,
  FIELD_COMMENT_COMMUNITY_NAME,
  FIELD_COMMENT_AUTHOR_ID,
  FIELD_COMMENT_AUTHOR_NAME,
  FIELD_COMMENT_BODY,
  FIELD_COMMENT_POINTS,
  FIELD_COMMENT_DATE_POSTED
};

static const int comments_field_lengths[COMMENTS_NUM_FIELDS] =
{
  INT_BUF_LEN,
  INT_BUF_LEN,
  POST_TITLE_BUF_LEN,
  INT_BUF_LEN,
  COMMUNITY_NAME_BUF_LEN,
  INT_BUF_LEN,
  USER_NAME_BUF_LEN,
  COMMENT_BODY_BUF_LEN,
  INT_BUF_LEN,
  INT_BUF_LEN,
};

static const struct table_info comments_table_info =
{
  comments_field_table,
  comments_field_lengths
};

static MYSQL_STMT* stmt_query_comment_by_id = NULL;
ks_hashmap* query_comment_by_id(const char* comment_id)
{
  ks_list* ls;
  if ((ls = sql_select(&comments_table_info, stmt_query_comment_by_id, comment_id)) == NULL)
  {
    return NULL;
  }

  ks_datacont* dc = ks_list_get(ls, 0);
  ks_hashmap* row0 = dc->hm;
  dc->hm = NULL;
  ks_list_delete(ls);

  return row0;
}

static MYSQL_STMT* stmt_query_comments_by_author_name = NULL;
ks_list* query_comments_by_author_name(const char* author_name)
{
  return sql_select(&comments_table_info, stmt_query_comments_by_author_name, author_name);
}

static MYSQL_STMT* stmt_query_comments_by_post_id = NULL;
ks_list* query_comments_by_post_id(const char* post_id)
{
  return sql_select(&comments_table_info, stmt_query_comments_by_post_id, post_id);
}

static MYSQL_STMT* stmt_create_comment = NULL;
char* sql_create_comment(const char* user_id, const char* post_id, const char* body)
{
  return sql_function(stmt_create_comment, user_id, post_id, body);
}

static MYSQL_STMT* stmt_update_comment_body = NULL;
int sql_update_comment_body(const char* comment_id, const char* body)
{
  return sql_procedure(stmt_update_comment_body, comment_id, body);
}

static MYSQL_STMT* stmt_delete_comment = NULL;
int sql_delete_comment(const char* comment_id)
{
  return sql_procedure(stmt_delete_comment, comment_id);
}


/**************************/
/* communities table info */

static const char* communities_field_table[COMMUNITIES_NUM_FIELDS] =
{
  FIELD_COMMUNITY_ID,
  FIELD_COMMUNITY_OWNER_ID,
  FIELD_COMMUNITY_OWNER_NAME,
  FIELD_COMMUNITY_NAME,
  FIELD_COMMUNITY_ABOUT,
  FIELD_COMMUNITY_MEMBERS,
  FIELD_COMMUNITY_DATE_CREATED
};

static const int communities_field_lengths[COMMUNITIES_NUM_FIELDS] =
{
  INT_BUF_LEN,
  INT_BUF_LEN,
  USER_NAME_BUF_LEN,
  COMMUNITY_NAME_BUF_LEN,
  COMMUNITY_ABOUT_BUF_LEN,
  INT_BUF_LEN,
  INT_BUF_LEN
};

static const struct table_info communities_table_info =
{
  communities_field_table,
  communities_field_lengths
};

static MYSQL_STMT* stmt_query_community_by_id = NULL;
ks_hashmap* query_community_by_id(const char* community_id)
{
  ks_list* ls;
  if ((ls = sql_select(&communities_table_info, stmt_query_community_by_id, community_id)) == NULL)
  {
    return NULL;
  }

  ks_datacont* dc = ks_list_get(ls, 0);
  ks_hashmap* row0 = dc->hm;
  dc->hm = NULL;
  ks_list_delete(ls);

  return row0;
}

static MYSQL_STMT* stmt_query_community_by_name = NULL;
ks_hashmap* query_community_by_name(const char* community_name)
{
  ks_list* ls;
  if ((ls = sql_select(&communities_table_info, stmt_query_community_by_name, community_name)) == NULL)
  {
    return NULL;
  }

  ks_datacont* dc = ks_list_get(ls, 0);
  ks_hashmap* row0 = dc->hm;
  dc->hm = NULL;
  ks_list_delete(ls);

  return row0;
}

static MYSQL_STMT* stmt_query_all_communities = NULL;
ks_list* query_all_communities()
{
  return sql_select(&communities_table_info, stmt_query_all_communities);
}

static MYSQL_STMT* stmt_create_community = NULL;
char* sql_create_community(const char* user_id, const char* community_name, const char* about)
{
  return sql_function(stmt_create_community, user_id, community_name, about);
}

static MYSQL_STMT* stmt_update_community_about = NULL;
int sql_update_community_about(const char* community_id, const char* about)
{
  return sql_procedure(stmt_update_community_about, community_id, about);
}

static MYSQL_STMT* stmt_delete_community = NULL;
int sql_delete_community(const char* community_id)
{
  return sql_procedure(stmt_delete_community, community_id);
}


/*************************/
/* moderators table info */

static const char* moderators_field_table[MODERATORS_NUM_FIELDS] =
{
  FIELD_MODERATOR_ID,
  FIELD_MODERATOR_USER_ID,
  FIELD_MODERATOR_COMMUNITY_ID
};

static const int moderators_field_lengths[MODERATORS_NUM_FIELDS] =
{
  INT_BUF_LEN,
  INT_BUF_LEN,
  INT_BUF_LEN
};

static const struct table_info moderators_table_info =
{
  moderators_field_table,
  moderators_field_lengths
};

static MYSQL_STMT* stmt_query_moderator_by_community_id_user_id = NULL;
ks_hashmap* query_moderator_by_community_id_user_id(const char* community_id, const char* user_id)
{
  ks_list* ls;
  if ((ls = sql_select(&moderators_table_info, stmt_query_moderator_by_community_id_user_id, community_id, user_id)) == NULL)
  {
    return NULL;
  }

  ks_datacont* dc = ks_list_get(ls, 0);
  ks_hashmap* row0 = dc->hm;
  dc->hm = NULL;
  ks_list_delete(ls);

  return row0;
}

static MYSQL_STMT* stmt_create_moderator = NULL;
char* sql_create_moderator(const char* user_id, const char* community_id)
{
  return sql_function(stmt_create_moderator, user_id, community_id);
}

static MYSQL_STMT* stmt_delete_moderator = NULL;
int sql_delete_moderator(const char* user_id, const char* community_id)
{
  return sql_procedure(stmt_delete_moderator, user_id, community_id);
}


/**************************/
/* adminstrators table info */

static const char* administrators_field_table[ADMINISTRATORS_NUM_FIELDS] =
{
  FIELD_ADMINISTRATOR_USER_ID
};

static const int administrators_field_lengths[ADMINISTRATORS_NUM_FIELDS] =
{
  INT_BUF_LEN
};

static const struct table_info administrators_table_info =
{
  administrators_field_table,
  administrators_field_lengths
};

static MYSQL_STMT* stmt_query_administrator_by_user_id = NULL;
ks_hashmap* query_administrator_by_user_id(const char* user_id)
{
  ks_list* ls;
  if ((ls = sql_select(&administrators_table_info, stmt_query_administrator_by_user_id, user_id)) == NULL)
  {
    return NULL;
  }

  ks_datacont* dc = ks_list_get(ls, 0);
  ks_hashmap* row0 = dc->hm;
  dc->hm = NULL;
  ks_list_delete(ls);

  return row0;
}

static MYSQL_STMT* stmt_create_administrator = NULL;
char* sql_create_administrator(const char* user_id)
{
  return sql_function(stmt_create_administrator, user_id);
}

static MYSQL_STMT* stmt_delete_administrator = NULL;
int sql_delete_administrator(const char* user_id)
{
  return sql_procedure(stmt_delete_administrator, user_id);
}


/****************************/
/* subscriptions table info */

static const char* subscriptions_field_table[SUBSCRIPTIONS_NUM_FIELDS] =
{
  FIELD_SUB_ID,
  FIELD_SUB_USER_ID,
  FIELD_SUB_COMMUNITY_ID
};

static const int subscriptions_field_lengths[SUBSCRIPTIONS_NUM_FIELDS] =
{
  INT_BUF_LEN,
  INT_BUF_LEN,
  INT_BUF_LEN
};

static const struct table_info subscriptions_table_info =
{
  subscriptions_field_table,
  subscriptions_field_lengths
};

static MYSQL_STMT* stmt_query_subscription_by_community_id_user_id = NULL;
ks_hashmap* query_subscription_by_community_id_user_id(const char* community_id, const char* user_id)
{
  ks_list* ls;
  if ((ls = sql_select(&subscriptions_table_info, stmt_query_subscription_by_community_id_user_id, community_id, user_id)) == NULL)
  {
    return NULL;
  }

  ks_datacont* dc = ks_list_get(ls, 0);
  ks_hashmap* row0 = dc->hm;
  dc->hm = NULL;
  ks_list_delete(ls);

  return row0;
}

static MYSQL_STMT* stmt_query_subscriptions_by_user_id = NULL;
ks_list* query_subscriptions_by_user_id(const char* user_id)
{
  return sql_select(&subscriptions_table_info, stmt_query_subscriptions_by_user_id, user_id);
}

static MYSQL_STMT* stmt_toggle_subscription = NULL;
int sql_toggle_subscription(const char* community_id, const char* user_id)
{
  return sql_procedure(stmt_toggle_subscription, community_id, user_id);
}


/*****************/
/* post up votes */

static const char* post_up_votes_field_table[POST_UP_VOTES_NUM_FIELDS] =
{
  FIELD_POST_UP_VOTE_ID,
  FIELD_POST_UP_VOTE_POST_ID,
  FIELD_POST_UP_VOTE_USER_ID
};

static const int post_up_votes_field_lengths[POST_UP_VOTES_NUM_FIELDS] =
{
  INT_BUF_LEN,
  INT_BUF_LEN,
  INT_BUF_LEN
};

static const struct table_info post_up_votes_table_info =
{
  post_up_votes_field_table,
  post_up_votes_field_lengths
};

static MYSQL_STMT* stmt_query_post_up_vote_by_post_id_user_id = NULL;
ks_hashmap* query_post_up_vote_by_post_id_user_id(const char* post_id, const char* user_id)
{
  ks_list* ls;
  if ((ls = sql_select(&post_up_votes_table_info, stmt_query_post_up_vote_by_post_id_user_id, post_id, user_id)) == NULL)
  {
    return NULL;
  }

  ks_datacont* dc = ks_list_get(ls, 0);
  ks_hashmap* row0 = dc->hm;
  dc->hm = NULL;
  ks_list_delete(ls);

  return row0;
}

static MYSQL_STMT* stmt_toggle_post_up_vote = NULL;
int sql_toggle_post_up_vote(const char* post_id, const char* user_id)
{
  return sql_procedure(stmt_toggle_post_up_vote, post_id, user_id);
}


/*******************/
/* post down votes */

static const char* post_down_votes_field_table[POST_DOWN_VOTES_NUM_FIELDS] =
{
  FIELD_POST_DOWN_VOTE_ID,
  FIELD_POST_DOWN_VOTE_POST_ID,
  FIELD_POST_DOWN_VOTE_USER_ID
};

static const int post_down_votes_field_lengths[POST_DOWN_VOTES_NUM_FIELDS] =
{
  INT_BUF_LEN,
  INT_BUF_LEN,
  INT_BUF_LEN
};

static const struct table_info post_down_votes_table_info =
{
  post_down_votes_field_table,
  post_down_votes_field_lengths
};

static MYSQL_STMT* stmt_query_post_down_vote_by_post_id_user_id = NULL;
ks_hashmap* query_post_down_vote_by_post_id_user_id(const char* post_id, const char* user_id)
{
  ks_list* ls;
  if ((ls = sql_select(&post_down_votes_table_info, stmt_query_post_down_vote_by_post_id_user_id, post_id, user_id)) == NULL)
  {
    return NULL;
  }

  ks_datacont* dc = ks_list_get(ls, 0);
  ks_hashmap* row0 = dc->hm;
  dc->hm = NULL;
  ks_list_delete(ls);

  return row0;
}

static MYSQL_STMT* stmt_toggle_post_down_vote = NULL;
int sql_toggle_post_down_vote(const char* post_id, const char* user_id)
{
  return sql_procedure(stmt_toggle_post_down_vote, post_id, user_id);
}


/********************/
/* comment up votes */

static const char* comment_up_votes_field_table[COMMENT_UP_VOTES_NUM_FIELDS] =
{
  FIELD_COMMENT_UP_VOTE_ID,
  FIELD_COMMENT_UP_VOTE_POST_ID,
  FIELD_COMMENT_UP_VOTE_USER_ID
};

static const int comment_up_votes_field_lengths[COMMENT_UP_VOTES_NUM_FIELDS] =
{
  INT_BUF_LEN,
  INT_BUF_LEN,
  INT_BUF_LEN
};

static const struct table_info comment_up_votes_table_info =
{
  comment_up_votes_field_table,
  comment_up_votes_field_lengths
};

static MYSQL_STMT* stmt_query_comment_up_vote_by_comment_id_user_id = NULL;
ks_hashmap* query_comment_up_vote_by_comment_id_user_id(const char* comment_id, const char* user_id)
{
  ks_list* ls;
  if ((ls = sql_select(&comment_up_votes_table_info, stmt_query_comment_up_vote_by_comment_id_user_id, comment_id, user_id)) == NULL)
  {
    return NULL;
  }

  ks_datacont* dc = ks_list_get(ls, 0);
  ks_hashmap* row0 = dc->hm;
  dc->hm = NULL;
  ks_list_delete(ls);

  return row0;
}

static MYSQL_STMT* stmt_toggle_comment_up_vote = NULL;
int sql_toggle_comment_up_vote(const char* comment_id, const char* user_id)
{
  return sql_procedure(stmt_toggle_comment_up_vote, comment_id, user_id);
}


/**********************/
/* comment down votes */

static const char* comment_down_votes_field_table[COMMENT_DOWN_VOTES_NUM_FIELDS] =
{
  FIELD_COMMENT_DOWN_VOTE_ID,
  FIELD_COMMENT_DOWN_VOTE_POST_ID,
  FIELD_COMMENT_DOWN_VOTE_USER_ID
};

static const int comment_down_votes_field_lengths[COMMENT_DOWN_VOTES_NUM_FIELDS] =
{
  INT_BUF_LEN,
  INT_BUF_LEN,
  INT_BUF_LEN
};

static const struct table_info comment_down_votes_table_info =
{
  comment_down_votes_field_table,
  comment_down_votes_field_lengths
};

static MYSQL_STMT* stmt_query_comment_down_vote_by_comment_id_user_id = NULL;
ks_hashmap* query_comment_down_vote_by_comment_id_user_id(const char* comment_id, const char* user_id)
{
  ks_list* ls;
  if ((ls = sql_select(&comment_down_votes_table_info, stmt_query_comment_down_vote_by_comment_id_user_id, comment_id, user_id)) == NULL)
  {
    return NULL;
  }

  ks_datacont* dc = ks_list_get(ls, 0);
  ks_hashmap* row0 = dc->hm;
  dc->hm = NULL;
  ks_list_delete(ls);

  return row0;
}

static MYSQL_STMT* stmt_toggle_comment_down_vote = NULL;
int sql_toggle_comment_down_vote(const char* comment_id, const char* user_id)
{
  return sql_procedure(stmt_toggle_comment_down_vote, comment_id, user_id);
}


static MYSQL* sqlcon;


static MYSQL_STMT* build_prepared_statement(MYSQL* sqlcon, const char* statement)
{
  // create statement object
  MYSQL_STMT* stmt;
  if ((stmt = mysql_stmt_init(sqlcon)) == NULL)
  {
    log_crit("build_prepared_statement(): mysql_stmt_init(): out of memory");
  }

  // prepare with statement string
  if (mysql_stmt_prepare(stmt, statement, strlen(statement)) != 0)
  {
    log_crit("build_prepared_statement(): mysql_stmt_prepare(): %s", mysql_stmt_error(stmt));
  }

  return stmt;
}


void init_sql_manager()
{
  log_info("Initializing SQL Manager...");

  // load db.config
  char* config;
  if ((config = load_file("backend/db.config")) == NULL)
  {
    log_crit("init_sql_manager(): failed to load db.config");
  }

  ks_hashmap* args = string_to_map(config, "\n", "=");
  free(config);

  const char* dbhost;
  if ((dbhost = get_map_value_str(args, "DBHOST")) == NULL)
  {
    ks_hashmap_delete(args);
    log_crit("init_sql_manager(): missing DBHOST parameter");
  }

  const char* dbname;
  if ((dbname = get_map_value_str(args, "DBNAME")) == NULL)
  {
    ks_hashmap_delete(args);
    log_crit("init_sql_manager(): missing DBNAME parameter");
  }

  const char* dbuser;
  if ((dbuser = get_map_value_str(args, "DBUSER")) == NULL)
  {
    ks_hashmap_delete(args);
    log_crit("init_sql_manager(): missing DBUSER paramter");
  }

  const char* dbpass;
  if ((dbpass = get_map_value_str(args, "DBPASS")) == NULL)
  {
    ks_hashmap_delete(args);
    log_crit("init_sql_manager(): missing DBPASS parameter");
  }

  // initialize sql connection object
  if ((sqlcon = mysql_init(NULL)) == NULL)
  {
    ks_hashmap_delete(args);
    log_crit("init_sql_manager(): mysql_init(): %s", mysql_error(NULL));
  }

  // connect to mysql database
  if (mysql_real_connect(sqlcon, dbhost, dbuser, dbpass, dbname, 0, NULL, 0) == NULL)
  {
    ks_hashmap_delete(args);
    log_crit("init_sql_manager(): mysql_real_connect(): %s", mysql_error(sqlcon));
  }

  ks_hashmap_delete(args);

  // build prepared statements

  // users
  stmt_query_user_by_id = build_prepared_statement(sqlcon, "SELECT * FROM users WHERE id = ?;");
  stmt_query_user_by_name = build_prepared_statement(sqlcon, "SELECT * FROM users WHERE name = ?;");
  stmt_create_user = build_prepared_statement(sqlcon, "SELECT CreateUser(?, ?, ?);");
  stmt_update_user_about = build_prepared_statement(sqlcon, "UPDATE users SET about = ? WHERE id = ?;");
  stmt_update_user_password_hash = build_prepared_statement(sqlcon, "UPDATE users SET password_hash = ? WHERE id = ?;");
  stmt_delete_user = build_prepared_statement(sqlcon, "CALL DeleteUser(?);");

  // posts
  stmt_query_post_by_id = build_prepared_statement(sqlcon, "SELECT * FROM posts WHERE id = ?;");
  stmt_query_all_posts = build_prepared_statement(sqlcon, "SELECT * FROM posts;");
  stmt_query_posts_by_author_name = build_prepared_statement(sqlcon, "SELECT * FROM posts WHERE author_name = ?;");
  stmt_query_posts_by_community_id = build_prepared_statement(sqlcon, "SELECT * FROM posts WHERE community_id = ?;");
  stmt_query_posts_by_community_name = build_prepared_statement(sqlcon, "SELECT * FROM posts WHERE community_name = ?;");
  stmt_create_post = build_prepared_statement(sqlcon, "SELECT CreatePost(?, ?, ?, ?);");
  stmt_update_post_body = build_prepared_statement(sqlcon, "UPDATE posts SET body = ? WHERE id = ?;");
  stmt_delete_post = build_prepared_statement(sqlcon, "CALL DeletePost(?);");

  // comments
  stmt_query_comment_by_id = build_prepared_statement(sqlcon, "SELECT * FROM comments WHERE id = ?;");
  stmt_query_comments_by_author_name = build_prepared_statement(sqlcon, "SELECT * FROM comments WHERE author_name = ?;");
  stmt_query_comments_by_post_id = build_prepared_statement(sqlcon, "SELECT * FROM comments WHERE post_id = ?;");
  stmt_create_comment = build_prepared_statement(sqlcon, "SELECT CreateComment(?, ?, ?);");
  stmt_update_comment_body = build_prepared_statement(sqlcon, "UPDATE comments SET body = ? WHERE id = ?;");
  stmt_delete_comment = build_prepared_statement(sqlcon, "CALL DeleteComment(?);");

  // communities
  stmt_query_community_by_id = build_prepared_statement(sqlcon, "SELECT * FROM communities WHERE id = ?;");
  stmt_query_community_by_name = build_prepared_statement(sqlcon, "SELECT * FROM communities WHERE name = ?;");
  stmt_query_all_communities = build_prepared_statement(sqlcon, "SELECT * FROM communities;");
  stmt_create_community = build_prepared_statement(sqlcon, "SELECT CreateCommunity(?, ?, ?);");
  stmt_update_community_about = build_prepared_statement(sqlcon, "UPDATE communities SET about = ? WHERE id = ?;");
  stmt_delete_community = build_prepared_statement(sqlcon, "CALL DeleteCommunity(?);");

  // moderators
  stmt_query_moderator_by_community_id_user_id = build_prepared_statement(sqlcon, "SELECT * FROM moderators WHERE community_id = ? AND user_id = ?;");
  stmt_create_moderator = build_prepared_statement(sqlcon, "INSERT INTO moderators (user_id, community_id) VALUES (?, ?);");
  stmt_delete_moderator = build_prepared_statement(sqlcon, "DELETE FROM moderators WHERE community_id = ? AND user_id = ?;");

  // administrators
  stmt_query_administrator_by_user_id = build_prepared_statement(sqlcon, "SELECT * FROM administrators WHERE user_id = ?;");
  stmt_create_administrator = build_prepared_statement(sqlcon, "INSERT INTO administrators (user_id) VALUES (?);");
  stmt_delete_administrator = build_prepared_statement(sqlcon, "DELETE FROM administrators WHERE user_id = ?;");

  // subscriptions
  stmt_query_subscription_by_community_id_user_id = build_prepared_statement(sqlcon, "SELECT * FROM subscriptions WHERE community_id = ? AND user_id = ?;");
  stmt_query_subscriptions_by_user_id = build_prepared_statement(sqlcon, "SELECT * FROM subscriptions WHERE user_id = ?;");
  stmt_toggle_subscription = build_prepared_statement(sqlcon, "CALL ToggleSubscription(?, ?);");

  // post votes
  stmt_query_post_up_vote_by_post_id_user_id = build_prepared_statement(sqlcon, "SELECT * FROM post_up_votes WHERE post_id = ? AND user_id = ?;");
  stmt_query_post_down_vote_by_post_id_user_id = build_prepared_statement(sqlcon, "SELECT * FROM post_down_votes WHERE post_id = ? AND user_id = ?;");
  stmt_toggle_post_up_vote = build_prepared_statement(sqlcon, "CALL TogglePostUpVote(?, ?);");
  stmt_toggle_post_down_vote = build_prepared_statement(sqlcon, "CALL TogglePostDownVote(?, ?);");

  // comment votes
  stmt_query_comment_up_vote_by_comment_id_user_id = build_prepared_statement(sqlcon, "SELECT * FROM comment_up_votes WHERE comment_id = ? AND user_id = ?;");
  stmt_query_comment_down_vote_by_comment_id_user_id = build_prepared_statement(sqlcon, "SELECT * FROM comment_down_votes WHERE comment_id = ? AND user_id = ?;");
  stmt_toggle_comment_up_vote = build_prepared_statement(sqlcon, "CALL ToggleCommentUpVote(?, ?);");
  stmt_toggle_comment_down_vote = build_prepared_statement(sqlcon, "CALL ToggleCommentDownVote(?, ?);");

  atexit(&terminate_sql_manager);
}


static void terminate_sql_manager()
{
  log_info("Terminating SQL Manager...");

  // close prepared statements

  // users
  mysql_stmt_close(stmt_query_user_by_id);
  mysql_stmt_close(stmt_query_user_by_name);
  mysql_stmt_close(stmt_create_user);
  mysql_stmt_close(stmt_update_user_about);
  mysql_stmt_close(stmt_update_user_password_hash);
  mysql_stmt_close(stmt_delete_user);

  // posts
  mysql_stmt_close(stmt_query_post_by_id);
  mysql_stmt_close(stmt_query_all_posts);
  mysql_stmt_close(stmt_query_posts_by_author_name);
  mysql_stmt_close(stmt_query_posts_by_community_id);
  mysql_stmt_close(stmt_query_posts_by_community_name);
  mysql_stmt_close(stmt_create_post);
  mysql_stmt_close(stmt_update_post_body);
  mysql_stmt_close(stmt_delete_post);

  // comments
  mysql_stmt_close(stmt_query_comment_by_id);
  mysql_stmt_close(stmt_query_comments_by_author_name);
  mysql_stmt_close(stmt_query_comments_by_post_id);
  mysql_stmt_close(stmt_create_comment);
  mysql_stmt_close(stmt_update_comment_body);
  mysql_stmt_close(stmt_delete_comment);

  // communities
  mysql_stmt_close(stmt_query_community_by_id);
  mysql_stmt_close(stmt_query_community_by_name);
  mysql_stmt_close(stmt_query_all_communities);
  mysql_stmt_close(stmt_create_community);
  mysql_stmt_close(stmt_update_community_about);
  mysql_stmt_close(stmt_delete_community);

  // moderators
  mysql_stmt_close(stmt_query_moderator_by_community_id_user_id);
  mysql_stmt_close(stmt_create_moderator);
  mysql_stmt_close(stmt_delete_moderator);

  // administrators
  mysql_stmt_close(stmt_query_administrator_by_user_id);
  mysql_stmt_close(stmt_create_administrator);
  mysql_stmt_close(stmt_delete_administrator);

  // subscriptions
  mysql_stmt_close(stmt_query_subscription_by_community_id_user_id);
  mysql_stmt_close(stmt_query_subscriptions_by_user_id);
  mysql_stmt_close(stmt_toggle_subscription);

  // post votes
  mysql_stmt_close(stmt_query_post_up_vote_by_post_id_user_id);
  mysql_stmt_close(stmt_query_post_down_vote_by_post_id_user_id);
  mysql_stmt_close(stmt_toggle_post_up_vote);
  mysql_stmt_close(stmt_toggle_post_down_vote);

  // comment votes
  mysql_stmt_close(stmt_query_comment_up_vote_by_comment_id_user_id);
  mysql_stmt_close(stmt_query_comment_down_vote_by_comment_id_user_id);
  mysql_stmt_close(stmt_toggle_comment_up_vote);
  mysql_stmt_close(stmt_toggle_comment_down_vote);

  // close mysql connection
  mysql_close(sqlcon);
  mysql_server_end();
}


/* executes a prepared statement, returns the number of rows
    in the result set, if any. Returns -1 on error. */
static int sql_exec(MYSQL_STMT* stmt, va_list ap) 
{
  // build argument array
  int count = mysql_stmt_param_count(stmt);
  MYSQL_BIND args[count];
  unsigned long lengths[count];
  memset(args, 0, count * sizeof(MYSQL_BIND));

  for (int i = 0; i < count; i++)
  {
    char* curr = va_arg(ap, char*);
    if (curr == NULL)
    {
      // if the buffer type is null, nothing else needs to be set
      args[i].buffer_type = MYSQL_TYPE_NULL;
    }
    else
    {
      lengths[i] = strlen(curr);
      args[i].buffer_type = MYSQL_TYPE_STRING;
      args[i].buffer = curr;
      args[i].buffer_length = lengths[i] + 1; // length of buffer containing string
      args[i].length = &lengths[i]; // length of string
    }
  }

  // bind args to statement
  if (mysql_stmt_bind_param(stmt, args) != 0)
  {
    log_err("sql_exec(): mysql_stmt_bind_param(): %s", mysql_stmt_error(stmt));
    return -1;
  }

  // execute statement
  if (mysql_stmt_execute(stmt) != 0)
  {
    log_err("sql_exec(): mysql_stmt_execute(): %s", mysql_stmt_error(stmt));
    return -1;
  }

  // load result set
  if (mysql_stmt_store_result(stmt) != 0)
  {
    log_err("sql_exec(): mysql_stmt_store_result(): %s", mysql_stmt_error(stmt));
    return -1;
  }

  return mysql_stmt_num_rows(stmt);
}


/* This function submits SELECT statements and returns a result set in the form
   of a list of hashmaps. */
static ks_list* sql_select(const struct table_info* table_info, MYSQL_STMT* stmt, ...)
{
  // submit query to mysql
  va_list ap;
  va_start(ap, stmt);
  int num_rows = sql_exec(stmt, ap);
  va_end(ap);

  if (num_rows == -1)
  {
    return NULL;
  }

  if (num_rows == 0)
  {
    return NULL;
  }

  int num_cols = mysql_stmt_field_count(stmt);

  // prepare output buffers
  MYSQL_BIND output[num_cols];
  memset(output, 0, sizeof(output));

  bool is_null[num_cols];
  memset(is_null, 0, sizeof(is_null));

  unsigned long length[num_cols];
  memset(length, 0, sizeof(length));

  bool error[num_cols];
  memset(error, 0, sizeof(error));

  for (int i = 0; i < num_cols; i++)
  {
    output[i].buffer_type = MYSQL_TYPE_STRING;
    output[i].buffer_length = table_info->field_lengths[i];
    output[i].buffer = malloc(sizeof(char) * table_info->field_lengths[i]);
    output[i].is_null = &(is_null[i]);
    output[i].length = &(length[i]);
    output[i].error = &(error[i]);
  }

  // bind output buffers
  ks_list* result = NULL;
  if (mysql_stmt_bind_result(stmt, output) != 0)
  {
    log_err("sql_select(): mysql_stmt_bind_result(): %s", mysql_stmt_error(stmt));
    goto free_buffers;
  }

  // iterate through rows and store data into list
  result = ks_list_new();
  for (int i = 0; i < num_rows; i++)
  {
    // load values into output buffer
    int status = mysql_stmt_fetch(stmt);
    if (status != 0)
    {
      if (status == 1)
      {
        log_err("sql_select(): mysql_stmt_fetch(): %s", mysql_stmt_error(stmt));
        goto free_buffers;
      }
      else if (status == MYSQL_NO_DATA)
      {
        log_err("sql_select(): fewer rows received than expected (recieved: %d, expected: %d)", i+1, num_rows);
        goto free_buffers;
      }
      else if (status == MYSQL_DATA_TRUNCATED)
      {
        log_err("sql_select(): data truncated for row %d, table with field '%s'", i, table_info->field_table[0]);
      }
    }

    ks_hashmap* row = ks_hashmap_new(KS_CHARP, num_cols);
    ks_list_add(result, ks_datacont_new(row, KS_HASHMAP, num_cols));

    for (int j = 0; j < num_cols; j++)
    {
      const char* key_str = table_info->field_table[j];
      int key_len = strlen(key_str);
      const char* val_str = is_null[j] ? "" : output[j].buffer;
      int val_len = length[j];

      ks_datacont* key = ks_datacont_new(key_str, KS_CHARP, key_len);
      ks_datacont* val = ks_datacont_new(val_str, KS_CHARP, val_len);

      ks_hashmap_add(row, key, val);
    }
  }

  // free up data buffers
free_buffers:
  for (int i = 0; i < num_cols; i++)  
  {
    free(output[i].buffer);
  }

  return result;
}


/* This function calls stored procedures in mysql, and returns
   0 on success, -1 on failure. */
static int sql_procedure(MYSQL_STMT* stmt, ...)
{
  // submit query to mysql
  va_list ap;
  va_start(ap, stmt);
  int res = sql_exec(stmt, ap);
  va_end(ap);

  return res;
}


/* This function calls stored functions that create new entries for
   users, comments, posts, and other items. This will return the ID
   of the newly created item as a string, or NULL on failure. */
static char* sql_function(MYSQL_STMT* stmt, ...)
{
  // submit query to mysql
  va_list ap;
  va_start(ap, stmt);
  int res = sql_exec(stmt, ap);
  va_end(ap);

  if (res == -1)
  {
    return NULL;
  }

  unsigned long length;
  bool is_null;
  bool error;
  MYSQL_BIND bind;
  memset(&bind, 0, sizeof(bind));

  bind.buffer_type = MYSQL_TYPE_STRING;
  bind.buffer = malloc(sizeof(char) * INT_BUF_LEN);
  bind.buffer_length = INT_BUF_LEN;
  bind.is_null = &is_null;
  bind.length = &length;
  bind.error = &error;

  if (mysql_stmt_bind_result(stmt, &bind) != 0)
  {
    log_err("sql_function(): mysql_stmt_bind_result(): %s", mysql_stmt_error(stmt));
    free(bind.buffer);
    return NULL;
  }

  int status = mysql_stmt_fetch(stmt);

  if (status != 0)
  {
    if (status == 1)
    {
      log_err("sql_function(): mysql_stmt_fetch(): %s", mysql_stmt_error(stmt));
      free(bind.buffer);
      return NULL;
    }
    else if (status == MYSQL_NO_DATA)
    {
      log_err("sql_function(): no ID returned from stored function");
      free(bind.buffer);
      return NULL;
    }
    else if (status == MYSQL_DATA_TRUNCATED)
    {
      log_err("sql_function(): returned ID truncated");
    }
  }

  return bind.buffer;
}
