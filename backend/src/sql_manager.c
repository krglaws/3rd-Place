#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql.h>
#include <kylestructs.h>

#include <log_manager.h>
#include <load_file.h>
#include <string_map.h>
#include <sql_manager.h>


/*******************************/
/* Output value buffer lengths
 *
 * the following
 * macros are equal to the maximum permitted
 * number of chars per value, plus one to
 * make room for the NULL terminator char.
 */

// used for all int-type values
#define INT_BUF_LEN (12)

// user specific
#define USER_NAME_BUF_LEN (17)
#define USER_PASSWD_BUF_LEN (129)
#define USER_ABOUT_BUF_LEN (257)

// post
#define POST_TITLE_BUF_LEN (33)
#define POST_BODY_BUF_LEN (513)

// comment
#define COMMENT_BODY_BUF_LEN (513)

// community
#define COMMUNITY_NAME_BUF_LEN (33)
#define COMMUNITY_ABOUT_BUF_LEN (512)

// one of these structs for each table in sql
// is used for retrieving data and storing for
// processing.
struct table_info
{
  int num_fields;
  const char** field_table;
  const int* field_lengths;
};

/********************/
/* users table info */

const char* users_field_table[USERS_NUM_FIELDS] =
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

const int users_field_lengths[USERS_NUM_FIELDS] =
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
  USERS_NUM_FIELDS,
  users_field_table,
  users_field_lengths
};

// user queries and their prepared statements
static MYSQL_STMT* stmt_query_users_by_name;
ks_list* query_users_by_name(const char* user_name)
{
  return sql_select(users_field_table, QUERY_USERS_BY_NAME, user_name);
}

static MYSQL_STMT* stmt_create_user;
char* sql_create_user(const char* user_name, const char* passwd_hash, const char* about)
{
  return sql_function(CREATE_USER, user_name, passwd_hash, about);
}

static MYSQL_STMT* stmt_delete_user;
int sql_delete_user(const char* user_id)
{
  return sql_procedure(
}

static void init_user_query_data()
{
  stmt_query_users_by_name = build_prepared_statment(sqlcon, "SELECT * FROM users WHERE name = ?;");
  stmt_create_user = build_prepared_statemtnt(sqlcon, "SELECT CreateUser(?, ?, ?);");
  stmt_delete_user = build_prepared_statemtnt(sqlcon, "CALL DeleteUser(?);");

  users_output_buffer[
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

#define QUERY_ALL_POSTS "SELECT * FROM posts;"
static MYSQL_STMT* stmt_query_all_posts;

ks_list* query_all_posts()
{
  return sql_select(posts_field_table, QUERY_ALL_POSTS);
}

#define QUERY_POSTS_BY_ID "SELECT * FROM posts WHERE id = %s;"
static MYSQL_STMT* stmt_query_posts_by_id;

ks_list* query_posts_by_id(const char* id)
{
  return sql_select(posts_field_table, QUERY_POSTS_BY_ID, id);
}

#define QUERY_POSTS_BY_AUTHOR_NAME "SELECT * FROM posts WHERE author_name = '%s';"
static MYSQL_STMT* stmt_query_posts_by_author_name;

ks_list* query_posts_by_author_name(const char* author_name)
{
  return sql_select(posts_field_table, QUERY_POSTS_BY_AUTHOR_NAME, author_name);
}

#define QUERY_POSTS_BY_COMMUNITY_ID "SELECT * FROM posts WHERE community_id = %s;"
static MYSQL_STMT* stmt_query_posts_by_community_id;

ks_list* query_posts_by_community_id(const char* community_id)
{
  return sql_select(posts_field_table, QUERY_POSTS_BY_COMMUNITY_ID, community_id);
}

#define QUERY_POSTS_BY_COMMUNITY_NAME "SELECT * FROM posts WHERE community_name = '%s';"
static MYSQL_STMT* stmt_query_posts_by_community_name;

ks_list* query_posts_by_community_name(const char* community_name)
{
  return sql_select(posts_field_table, QUERY_POSTS_BY_COMMUNITY_NAME, community_name);
}

#define CREATE_POST "SELECT CreatePost(%s, %s, '%s', '%s');"
static MYSQL_STMT* stmt_create_post;

char* sql_create_post(const char* user_id, const char* community_id, const char* title, const char* body)
{
  return sql_function(CREATE_POST, user_id, community_id, title, body);
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

#define QUERY_COMMENTS_BY_ID "SELECT * FROM comments WHERE comment_id = %s;"
static MYSQL_STMT* stmt_query_comments_by_id;

ks_list* query_comments_by_id(const char* comment_id)
{
  return sql_select(comments_field_table, QUERY_COMMENTS_BY_ID, comment_id);
}

#define QUERY_COMMENTS_BY_AUTHOR_NAME "SELECT * FROM comments WHERE author_name = '%s';"
static MYSQL_STMT* stmt_query_comments_by_author_name;

ks_list* query_comments_by_author_name(const char* author_name)
{
  return sql_select(comments_field_table, QUERY_COMMENTS_BY_AUTHOR_NAME, author_name);
}

#define QUERY_COMMENTS_BY_POST_ID "SELECT * FROM comments WHERE post_id = %s;"
static MYSQL_STMT* stmt_query_comments_by_post_id;

ks_list* query_comments_by_post_id(const char* post_id)
{
  return sql_select(comments_field_table, QUERY_COMMENTS_BY_POST_ID, post_id);
}

#define CREATE_COMMENT "SELECT CreateComment(%s, %s, %s, '%s');"
static MYSQL_STMT* stmt_create_comment;

char* sql_create_comment(const char* user_id, const char* post_id, const char* community_id, const char* body)
{
  return sql_function(CREATE_COMMENT, user_id, post_id, community_id, body);
}



/**************************/
/* communities table info */

static const char* communities_field_table[COMMUNITIES_NUM_FIELDS] =
{
  FIELD_COMMUNITY_ID,
  FIELD_COMMUNITY_OWNER_ID,
  FIELD_COMMUNITY_NAME,
  FIELD_COMMUNITY_ABOUT,
  FIELD_COMMUNITY_MEMBERS,
  FIELD_COMMUNITY_DATE_CREATED
};

#define QUERY_ALL_COMMUNITIES "SELECT * FROM communities;"
static MYSQL_STMT* stmt_query_all_communities;

ks_list* query_all_communities()
{
  return sql_select(communities_field_table, QUERY_ALL_COMMUNITIES);
}

#define QUERY_COMMUNITIES_BY_NAME "SELECT * FROM communities WHERE name = '%s';"
static MYSQL_STMT* stmt_query_communities_by_name;

ks_list* query_communities_by_name(const char* community_name)
{
  return sql_select(communities_field_table, QUERY_COMMUNITIES_BY_NAME, community_name);
}


/**************************/
/* moderators table info */

static const char* moderators_field_table[MODERATORS_NUM_FIELDS] =
{
  FIELD_MODERATOR_ID,
  FIELD_MODERATOR_USER_ID,
  FIELD_MODERATOR_COMMUNITY_ID
};

#define QUERY_MODERATORS_BY_COMMUNITY_ID_USER_ID "SELECT * FROM moderators WHERE community_id = %s AND user_id = %s;"
static MYSQL_STMT* stmt_query_moderators_by_community_id_user_id;

ks_list* query_moderators_by_community_id_user_id(const char* community_id, const char* user_id)
{
  return sql_select(moderators_field_table, QUERY_MODERATORS_BY_COMMUNITY_ID_USER_ID, community_id, user_id);
}


/**************************/
/* adminstrators table info */

static const char* administrators_field_table[ADMINISTRATORS_NUM_FIELDS] =
{
  FIELD_ADMINISTRATOR_USER_ID
};

#define QUERY_ADMINISTRATORS_BY_USER_ID "SELECT * FROM administrators WHERE user_id = %s;"
static MYSQL_STMT* stmt_query_administrators_by_user_id(const char* user_id);

ks_list* query_administrators_by_user_id(const char* user_id)
{
  return sql_select(administrators_field_table, QUERY_ADMINISTRATORS_BY_USER_ID, user_id);
}


/****************************/
/* subscriptions table info */

static const char* subscriptions_field_table[SUBSCRIPTIONS_NUM_FIELDS] =
{
  FIELD_SUB_ID,
  FIELD_SUB_USER_ID,
  FIELD_SUB_COMMUNITY_ID
};

#define QUERY_SUBSCRIPTIONS_BY_COMMUNITY_ID_USER_ID "SELECT * FROM subscriptions WHERE community_id = %s AND user_id = %s;"
static MYSQL_STMT* stmt_query_subscriptions_by_community_id_user_id;

ks_list* query_subscriptions_by_community_id_user_id(const char* community_id, const char* user_id)
{
  return sql_select(subscriptions_field_table, QUERY_SUBSCRIPTIONS_BY_COMMUNITY_ID_USER_ID, community_id, user_id);
}

#define QUERY_SUBSCRIPTIONS_BY_USER_ID "SELECT * FROM subscriptions WHERE user_id = %s;"
static MYSQL_STMT* stmt_query_subscriptions_by_user_id;

ks_list* query_subscriptions_by_user_id(const char* user_id)
{
  return sql_select(subscriptions_field_table, QUERY_SUBSCRIPTIONS_BY_USER_ID, user_id);
}

#define TOGGLE_SUBSCRIBE "CALL ToggleSubscribe(%s, %s);"
static MYSQL_STMT* stmt_toggle_subscribe;

int sql_toggle_subscribe(const char* community_id, const char* user_id)
{
  return sql_procedure(TOGGLE_SUBSCRIBE, community_id, user_id);
}


/*****************/
/* post up votes */

static const char* post_upvotes_field_table[POST_UP_VOTES_NUM_FIELDS] =
{
  FIELD_POST_UP_VOTE_ID,
  FIELD_POST_UP_VOTE_POST_ID,
  FIELD_POST_UP_VOTE_USER_ID
};

#define QUERY_POST_UP_VOTES_BY_POST_ID_USER_ID "SELECT * FROM post_up_votes WHERE post_id = %s AND user_id = %s;"
static MYSQL_STMT* stmt_query_post_upvotes_by_post_id_user_id(const char* post_id, const char* user_id);

ks_list* query_post_upvotes_by_post_id_user_id(const char* post_id, const char* user_id)
{
  return sql_select(post_upvotes_field_table, QUERY_POST_UP_VOTES_BY_POST_ID_USER_ID, post_id, user_id);
}

#define TOGGLE_POST_UP_VOTE "CALL TogglePostUpVote(%s, %s);"
static MYSQL_STMT* stmt_toggle_post_up_vote;

int sql_toggle_post_up_vote(const char* post_id, const char* user_id)
{
  return sql_procedure(TOGGLE_POST_UP_VOTE, post_id, user_id);
}


/*******************/
/* post down votes */

static const char* post_downvotes_field_table[POST_DOWN_VOTES_NUM_FIELDS] =
{
  FIELD_POST_DOWN_VOTE_ID,
  FIELD_POST_DOWN_VOTE_POST_ID,
  FIELD_POST_DOWN_VOTE_USER_ID
};

#define QUERY_POST_DOWN_VOTES_BY_POST_ID_USER_ID "SELECT * FROM post_down_votes WHERE post_id = %s AND user_id = %s;"
static MYSQL_STMT* stmt_query_post_downvotes_by_post_id_user_id(const char* post_id, const char* user_id);

ks_list* query_post_downvotes_by_post_id_user_id(const char* post_id, const char* user_id)
{
  return sql_select(post_downvotes_field_table, QUERY_POST_DOWN_VOTES_BY_POST_ID_USER_ID, post_id, user_id);
}

#define TOGGLE_POST_DOWN_VOTE "CALL TogglePostDownVote(%s, %s);"
static MYSQL_STMT* stmt_toggle_post_down_vote;

int sql_toggle_post_down_vote(const char* post_id, const char* user_id)
{
  return sql_procedure(TOGGLE_POST_DOWN_VOTE, post_id, user_id);
}


/********************/
/* comment up votes */

static const char* comment_upvotes_field_table[COMMENT_UP_VOTES_NUM_FIELDS] =
{
  FIELD_COMMENT_UP_VOTE_ID,
  FIELD_COMMENT_UP_VOTE_POST_ID,
  FIELD_COMMENT_UP_VOTE_USER_ID
};

#define QUERY_COMMENT_UP_VOTES_BY_COMMENT_ID_USER_ID "SELECT * FROM comment_up_votes WHERE comment_id = %s AND user_id = %s;"
static MYSQL_STMT* stmt_query_comment_upvotes_by_post_id_user_id(const char* comment_id, const char* user_id);

ks_list* query_comment_upvotes_by_post_id_user_id(const char* comment_id, const char* user_id)
{
  return sql_select(comment_downvotes_field_table, QUERY_COMMENT_UP_VOTES_BY_COMMENT_ID_USER_ID, comment_id, user_id);
}

#define TOGGLE_COMMENT_UP_VOTE "CALL ToggleCommentUpVote(%s, %s);"
static MYSQL_STMT* stmt_toggle_comment_up_vote;

int sql_toggle_comment_down_vote(const char* comment_id, const char* user_id)
{
  return sql_procedure(TOGGLE_COMMENT_DOWN_VOTE, comment_id, user_id);
}


/**********************/
/* comment down votes */

static const char* comment_downvotes_field_table[COMMENT_DOWN_VOTES_NUM_FIELDS] =
{
  FIELD_COMMENT_DOWN_VOTE_ID,
  FIELD_COMMENT_DOWN_VOTE_POST_ID,
  FIELD_COMMENT_DOWN_VOTE_USER_ID
};

#define QUERY_COMMENT_DOWN_VOTES_BY_COMMENT_ID_USER_ID "SELECT * FROM comment_down_votes WHERE comment_id = %s AND user_id = %s;"
static MYSQL_STMT* stmt_query_comment_downvotes_by_post_id_user_id;

ks_list* query_comment_downvotes_by_post_id_user_id(const char* comment_id, const char* user_id)
{
  return sql_select(post_downvotes_field_table, QUERY_COMMENT_DOWN_VOTES_BY_COMMENT_ID_USER_ID, comment_id, user_id);
}

#define TOGGLE_COMMENT_DOWN_VOTE "CALL ToggleCommentDownVote(%s, %s);"




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
    log_crit("init_sql_manager(): missing DBHOST parameter");
  }

  const char* dbname;
  if ((dbname = get_map_value_str(args, "DBNAME")) == NULL)
  {
    log_crit("init_sql_manager(): missing DBNAME parameter");
  }

  const char* dbuser;
  if ((dbuser = get_map_value_str(args, "DBUSER")) == NULL)
  {
    log_crit("init_sql_manager(): missing DBUSER paramter");
  }

  const char* dbpass;
  if ((dbpass = get_map_value_str(args, "DBPASS")) == NULL)
  {
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
  // posts
  stmt_query_posts_by_id = build_prepared_statement(sqlcon, "SELECT * FROM posts WHERE id = ?;");
  stmt_query_posts_by_author_name = build_prepared_statement(sqlcon, "SELECT * FROM posts WHERE author_name = ?;");
  stmt_query_posts_by_community_id = build_prepared_statement(sqlcon, "SELECT * FROM posts WHERE community_id = ?;");
  stmt_query_posts_by_community_name = build_prepared_statement(sqlcon, "SELECT * FROM posts WHERE community_name = ?;");
  stmt_create_post = build_prepared_statement(sqlcon, "SELECT CreatePost(?, ?, ?, ?);");
  stmt_delete_post = build_prepared_statement(sqlcon, "CALL DeletePost(?);");

  // comments
  stmt_query_comments_by_id = build_prepared_statement(sqlcon, "SELECT * FROM comments WHERE id = ?;");
  stmt_query_comments_by_author_name = build_prepared_statement(sqlcon, "SELECT * FROM comments WHERE author_name = ?;");
  stmt_query_comments_by_post_id = build_prepared_statement(sqlcon, "SELECT * FROM comments WHERE post_id = ?;");
  stmt_create_comment = build_prepared_statement(sqlcon, "SELECT CreateComment(?, ?, ?, ?);");
  stmt_delete_comment = build_prepared_statement(sqlcon, "CALL DeleteComment(?);");

  // communities
  stmt_query_communities_by_name = build_prepared_statement(sqlcon, "SELECT * FROM communities WHERE name = ?;");
  stmt_create_community = build_prepared_statement(sqlcon, "SELECT CreateCommunity(?, ?, ?);");
  stmt_delete_community = build_prepared_statement(sqlcon, "CALL DeleteCommunity(?);");

  // moderators
  stmt_query_moderators_by_community_id_user_id = build_prepared_statement(sqlcon, "SELECT * FROM moderators WHERE community_id = ? AND user_id = ?;");
  stmt_create_moderator = build_prepared_statement(sqlcon, "INSERT INTO moderators (user_id, community_id) VALUES (?, ?);");
  stmt_delete_moderator = build_prepared_statement(sqlcon, "DELETE FROM moderators WHERE user_id = ?;");

  // administrators
  stmt_query_administrators_by_user_id, build_prepared_statement(sqlcon, "SELECT * FROM administrators WHERE user_id = ?;");
  stmt_create_administrator = build_prepared_statement(sqlcon, "INSERT INTO administrators (user_id) VALUES (?);");
  stmt_delete_administrator = build_prepared_statement(sqlcon, "DELETE FROM administrators WHERE user_id = ?;");

  // subscriptions
  stmt_query_subscriptions_by_community_id_user_id = build_prepared_statement(sqlcon, "SELECT * FROM subscriptions WHERE community_id = ? AND user_id = ?;");
  stmt_query_subscriptions_by_user_id = build_prepared_statement(sqlcon, "SELECT * FROM subscriptions WHERE user_id = ?;");
  stmt_toggle_subscription = build_prepared_statement(sqlcon, "CALL ToggleSubscription(?, ?);");

  // post votes
  stmt_query_post_up_votes_by_post_id_user_id = build_prepared_statement(sqlcon, "SELECT * FROM post_up_votes WHERE post_id = ? AND user_id = ?;");
  stmt_query_post_down_votes_by_post_id_user_id = build_prepared_statement(sqlcon, "SELECT * FROM post_down_votes WHERE post_id = ? AND user_id = ?;");
  stmt_toggle_post_up_vote = build_prepared_statement(sqlcon, "CALL TogglePostUpVote(?, ?);");
  stmt_toggle_post_down_vote = build_prepared_statement(sqlcon, "CALL TogglePostDownVote(?, ?);");

  // comment votes
  stmt_query_comment_up_votes_by_comment_id_user_id = build_prepared_statement(sqlcon, "SELECT * FROM comment_up_votes WHERE comment_id = ? AND user_id = ?;");
  stmt_query_comment_down_votes_by_comment_id_user_id = build_prepared_statement(sqlcon, "SELECT * FROM comment_down_votes WHERE comment_id = ? AND user_id = ?;");
  stmt_toggle_comment_up_vote = build_prepared_statement(sqlcon, "CALL ToggleCommentUpVote(?, ?);");
  stmt_toggle_comment_down_vote = build_prepared_statement(sqlcon, "CALL ToggleCommentDownVote(?, ?);");
}


void terminate_sql_manager()
{
  log_info("Terminating SQL Manager...");

  // close prepared statements

  // users
  mysql_stmt_close(stmt_query_users_by_name);
  mysql_stmt_close(stmt_create_user);
  mysql_stmt_close(stmt_delete_user);

  // posts
  mysql_stmt_close(stmt_query_posts_by_id);
  mysql_stmt_close(stmt_query_posts_by_author_name);
  mysql_stmt_close(stmt_query_posts_by_community_id);
  mysql_stmt_close(stmt_query_posts_by_community_name);
  mysql_stmt_close(stmt_create_post);
  mysql_stmt_close(stmt_delete_post);

  // comments
  mysql_stmt_close(stmt_query_comments_by_id);
  mysql_stmt_close(stmt_query_comments_by_author_name);
  mysql_stmt_close(stmt_query_comments_by_post_id);
  mysql_stmt_close(stmt_create_comment);
  mysql_stmt_close(stmt_delete_comment);

  // communities
  mysql_stmt_close(stmt_query_communities_by_name);
  mysql_stmt_close(stmt_create_community);
  mysql_stmt_close(stmt_delete_community);

  // moderators
  mysql_stmt_close(stmt_query_moderators_by_community_id_user_id);
  mysql_stmt_close(stmt_create_moderator);
  mysql_stmt_close(stmt_delete_moderator);

  // administrators
  mysql_stmt_close(stmt_query_administrators_by_user_id);
  mysql_stmt_close(stmt_create_administrator);
  mysql_stmt_close(stmt_delete_administrator);

  // subscriptions
  mysql_stmt_close(stmt_query_subscriptions_by_community_id_user_id);
  mysql_stmt_close(stmt_query_subscriptions_by_user_id);
  mysql_stmt_close(stmt_toggle_subscription);

  // post votes
  mysql_stmt_close(stmt_query_post_up_votes_by_post_id_user_id);
  mysql_stmt_close(stmt_query_post_down_votes_by_post_id_user_id);
  mysql_stmt_close(stmt_toggle_post_up_vote);
  mysql_stmt_close(stmt_toggle_post_down_vote);

  // comment votes
  mysql_stmt_close(stmt_query_comment_up_votes_by_comment_id_user_id);
  mysql_stmt_close(stmt_query_comment_down_votes_by_comment_id_user_id);
  mysql_stmt_close(stmt_toggle_comment_up_vote);
  mysql_stmt_close(stmt_toggle_comment_down_vote);

  // close mysql connection

  mysql_close(sqlcon);
  mysql_server_end();
}


/* executes a prepared statement, returns the number of rows
    in the result set, if any. Returns -1 on error. */
static int sql_exec(MYSQL_STMT* stmt, va_list* ap) 
{
  // build argument array
  int count = mysql_stmt_param_count(stmt);
  MYSQL_BIND args[count];
  unsigned long lengths[count];
  memset(args, 0, count * sizeof(MYSQL_BIND));

  for (int i = 0; i < count; i++)
  {
    char* curr = va_arg(ap, char*);
    lengths[i] = strlen(curr);

    args[i].buffer_type = MYSQL_TYPE_STRING;
    args[i].buffer = curr;
    args[i].buffer_length = lengths[i] + 1; // length of buffer containing string
    args[i].length = &lengths[i]; // length of string
  }
  va_end(ap);

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
  int num_cols = mysql_stmt_field_count(stmt);

  if (num_rows == -1)
  {
    log_err("sql_select(): failed on call to sql_exec()");
    return NULL;
  }

  // prepare output buffers
  MYSQL_BIND output[num_cols];
  memset(output, 0, sizeof(output));

  int is_null[num_cols];
  memset(is_null, 0, sizeof(is_null));

  int length[num_cols];
  memset(length, 0, sizeof(length));

  int error[num_cols];
  memset(error, 0, sizeof(error));

  for (int i = 0; i < num_cols; i++)
  {
    output[i].buffer_type = MYSQL_TYPE_STRING;
    output[i].buffer = malloc(sizeof(char) * table_info->field_lengths[i]);
    output[i].is_null = &is_null[i];
    output[i].length = &length[i];
    output[i].error = &error[i];
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
    ks_hashmap* row = ks_hashmap_new(KS_CHARP, table_info->num_fields);
    ks_list_add(result, ks_datacont_new(row, KS_HASHMAP, table_info->num_fields));

    // load values into output buffer
    if (mysql_stmt_fetch(stmt) == 1)
    {
      log_crit("sql_select(): mysql_stmt_fetch(): %s", mysql_stmt_error(stmt));
    }

    for (int j = 0; j < table_info->num_fields; j++)
    {
      const char* key_str = table_info->field_table[i];
      int key_len = strlen(key_str);
      const char* val_str = is_null[i] ? "" : output[i].buffer;
      int val_len = length[i];

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

  return rows;
}


/* This function calls stored procedures in mysql, and returns
   0 on success, -1 on failure. */
static int sql_procedure(const char* tmplt, ...)
{
}


/* This function calls stored functions that create new entries for
   users, comments, posts, and other items. This will return the ID
   of the newly created item as a string, or NULL on failure. */
static char* sql_function(const char* tmplt, ...)
{
  char query[QUERYSTRLEN];

  // build query string
  va_list ap;
  va_start(ap, tmplt);
  vsprintf(query, tmplt, ap);
  va_end(ap);

  // submit query
  if (mysql_query(sqlcon, query) != 0)
  {
    log_err("sql_function(): mysql_query(): %s", mysql_error(sqlcon));
    return NULL;
  }

  // get result object
  MYSQL_RES* result;
  if ((result = mysql_store_result(sqlcon)) == NULL)
  {
    if (mysql_errno(sqlcon) != 0)
    {
      log_err("sql_function(): mysql_store_result(): %s", mysql_error(sqlcon));
    }   
    else
    {
      log_err("sql_function(): an unknown error occured. " \
              "mysql_store_result() returned NULL with no explaination.");
    }
    return NULL;
  }

  // grab what should be the only string from result
  MYSQL_ROW sqlrow = mysql_fetch_row(result);

  int len = strlen(sqlrow[0]);
  char* str = malloc(len + 1);
  memcpy(str, sqlrow[0], len + 1);

  mysql_free_result(result);

  return str;
}





char* sql_create_community(const char* user_id, const char* community_name, const char* about)
{
  return sql_function(CREATE_COMMUNITY, user_id, community_name, about);
}


int sql_delete_user(const char* user_id)
{
  return sql_procedure(DELETE_USER, user_id);
}


int sql_delete_comment(const char* comment_id)
{
  return sql_procedure(DELETE_COMMENT, comment_id);
}


int sql_delete_post(const char* post_id)
{
  return sql_procedure(DELETE_POST, post_id);
}


int sql_delete_community(const char* community_id)
{
  return sql_procedure(DELETE_COMMUNITY, community_id);
}
