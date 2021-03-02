#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql.h>
#include <kylestructs.h>

#include <log_manager.h>
#include <load_file.h>
#include <string_map.h>
#include <sql_manager.h>


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


static const char* communities_field_table[COMMUNITIES_NUM_FIELDS] =
{
  FIELD_COMMUNITY_ID,
  FIELD_COMMUNITY_OWNER_ID,
  FIELD_COMMUNITY_NAME,
  FIELD_COMMUNITY_ABOUT,
  FIELD_COMMUNITY_MEMBERS,
  FIELD_COMMUNITY_DATE_CREATED
};


static const char* moderators_field_table[MODERATORS_NUM_FIELDS] =
{
  FIELD_MODERATOR_ID,
  FIELD_MODERATOR_USER_ID,
  FIELD_MODERATOR_COMMUNITY_ID
};


static const char* administrators_field_table[ADMINISTRATORS_NUM_FIELDS] =
{
  FIELD_ADMINISTRATOR_USER_ID
};


static const char* subscriptions_field_table[SUBSCRIPTIONS_NUM_FIELDS] =
{
  FIELD_SUB_ID,
  FIELD_SUB_USER_ID,
  FIELD_SUB_COMMUNITY_ID
};


static const char* post_upvotes_field_table[POST_UP_VOTES_NUM_FIELDS] =
{
  FIELD_POST_UP_VOTE_ID,
  FIELD_POST_UP_VOTE_POST_ID,
  FIELD_POST_UP_VOTE_USER_ID
};


static const char* post_downvotes_field_table[POST_DOWN_VOTES_NUM_FIELDS] =
{
  FIELD_POST_DOWN_VOTE_ID,
  FIELD_POST_DOWN_VOTE_POST_ID,
  FIELD_POST_DOWN_VOTE_USER_ID
};


static const char* comment_upvotes_field_table[COMMENT_UP_VOTES_NUM_FIELDS] =
{
  FIELD_COMMENT_UP_VOTE_ID,
  FIELD_COMMENT_UP_VOTE_POST_ID,
  FIELD_COMMENT_UP_VOTE_USER_ID
};


static const char* comment_downvotes_field_table[COMMENT_DOWN_VOTES_NUM_FIELDS] =
{
  FIELD_COMMENT_DOWN_VOTE_ID,
  FIELD_COMMENT_DOWN_VOTE_POST_ID,
  FIELD_COMMENT_DOWN_VOTE_USER_ID
};


static MYSQL* sqlcon;


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
}


void terminate_sql_manager()
{
  log_info("Terminating SQL Manager...");

  mysql_close(sqlcon);
  mysql_server_end();
}


#define QUERYSTRLEN (1024)

/* This function submits SELECT statements and returns a result set in the form
   of a list of hashmaps. */
static ks_list* sql_select(const char** field_table, const char* tmplt, ...)
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
    log_err("sql_select(): mysql_query(): %s", mysql_error(sqlcon));
    return NULL;
  }

  // get result object
  MYSQL_RES* result;
  if ((result = mysql_store_result(sqlcon)) == NULL)
  {
    // if result is NULL, something must have gone wrong,
    // since this function should always expect a result set.
    // queries like INSERT, or CALL, which don't return result
    // sets, should use sql_function() or sql_procedure() below.
    if (mysql_errno(sqlcon) != 0)
    {
      log_err("sql_select(): mysql_store_result(): %s", mysql_error(sqlcon));
    }

    return NULL;
  }

  int num_rows;
  if ((num_rows = mysql_num_rows(result)) == 0)
  {
    // no rows returned
    return NULL;
  }

  int num_fields = mysql_num_fields(result);
  ks_list* rows = ks_list_new();

  // iterate through each row, store into list
  for (int i = 0; i < num_rows; i++)
  {
    char* field;
    ks_hashmap* row = ks_hashmap_new(KS_CHARP, num_fields * 2);
    MYSQL_ROW sqlrow = mysql_fetch_row(result);

    // iterate through each column, store into hashmap
    for (int j = 0; j < num_fields; j++)
    {
      const char* key_str = field_table[j];
      const char* value_str = sqlrow[j] ? sqlrow[j] : "";

      ks_datacont* key = ks_datacont_new(key_str, KS_CHARP, strlen(key_str));
      ks_datacont* value = ks_datacont_new(value_str, KS_CHARP, strlen(value_str));

      ks_hashmap_add(row, key, value);
    }

    ks_list_add(rows, ks_datacont_new(row, KS_HASHMAP, num_fields));
  }

  mysql_free_result(result);

  return rows;
}


/* This function calls stored procedures in mysql, and returns
   0 on success, -1 on failure. */
static int sql_procedure(const char* tmplt, ...)
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
    log_err("sql_procedure(): mysql_query(): %s", mysql_error(sqlcon));
    return -1;
  }

  return 0;
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


ks_list* query_users_by_name(const char* user_name)
{
  return sql_select(users_field_table, QUERY_USERS_BY_NAME, user_name);
}


ks_list* query_all_posts()
{
  return sql_select(posts_field_table, QUERY_ALL_POSTS);
}


ks_list* query_posts_by_id(const char* id)
{
  return sql_select(posts_field_table, QUERY_POSTS_BY_ID, id);
}


ks_list* query_posts_by_author_name(const char* author_name)
{
  return sql_select(posts_field_table, QUERY_POSTS_BY_AUTHOR_NAME, author_name);
}


ks_list* query_posts_by_community_id(const char* community_id)
{
  return sql_select(posts_field_table, QUERY_POSTS_BY_COMMUNITY_ID, community_id);
}


ks_list* query_posts_by_community_name(const char* community_name)
{
  return sql_select(posts_field_table, QUERY_POSTS_BY_COMMUNITY_NAME, community_name);
}


ks_list* query_comments_by_id(const char* comment_id)
{
  return sql_select(comments_field_table, QUERY_COMMENTS_BY_ID, comment_id);
}


ks_list* query_comments_by_author_name(const char* author_name)
{
  return sql_select(comments_field_table, QUERY_COMMENTS_BY_AUTHOR_NAME, author_name);
}


ks_list* query_comments_by_post_id(const char* post_id)
{
  return sql_select(comments_field_table, QUERY_COMMENTS_BY_POST_ID, post_id);
}


ks_list* query_all_communities()
{
  return sql_select(communities_field_table, QUERY_ALL_COMMUNITIES);
}


ks_list* query_communities_by_name(const char* community_name)
{
  return sql_select(communities_field_table, QUERY_COMMUNITIES_BY_NAME, community_name);
}


ks_list* query_moderators_by_community_id_user_id(const char* community_id, const char* user_id)
{
  return sql_select(moderators_field_table, QUERY_MODERATORS_BY_COMMUNITY_ID_USER_ID, community_id, user_id);
}


ks_list* query_administrators_by_user_id(const char* user_id)
{
  return sql_select(administrators_field_table, QUERY_ADMINISTRATORS_BY_USER_ID, user_id);
}


ks_list* query_subscriptions_by_user_id(const char* user_id)
{
  return sql_select(subscriptions_field_table, QUERY_SUBSCRIPTIONS_BY_USER_ID, user_id);
}


ks_list* query_post_upvotes_by_post_id_user_id(const char* post_id, const char* user_id)
{
  return sql_select(post_upvotes_field_table, QUERY_POST_UP_VOTES_BY_POST_ID_USER_ID, post_id, user_id);
}


ks_list* query_post_downvotes_by_post_id_user_id(const char* post_id, const char* user_id)
{
  return sql_select(post_downvotes_field_table, QUERY_POST_DOWN_VOTES_BY_POST_ID_USER_ID, post_id, user_id);
}


ks_list* query_comment_upvotes_by_post_id_user_id(const char* comment_id, const char* user_id)
{
  return sql_select(comment_downvotes_field_table, QUERY_COMMENT_UP_VOTES_BY_COMMENT_ID_USER_ID, comment_id, user_id);
}


ks_list* query_comment_downvotes_by_post_id_user_id(const char* comment_id, const char* user_id)
{
  return sql_select(post_downvotes_field_table, QUERY_COMMENT_DOWN_VOTES_BY_COMMENT_ID_USER_ID, comment_id, user_id);
}


ks_list* query_subscriptions_by_community_id_user_id(const char* community_id, const char* user_id)
{
  return sql_select(subscriptions_field_table, QUERY_SUBSCRIPTIONS_BY_COMMUNITY_ID_USER_ID, community_id, user_id);
}


int sql_toggle_subscribe(const char* community_id, const char* user_id)
{
  return sql_procedure(TOGGLE_SUBSCRIBE, community_id, user_id);
}


int sql_toggle_post_up_vote(const char* post_id, const char* user_id)
{
  return sql_procedure(TOGGLE_POST_UP_VOTE, post_id, user_id);
}


int sql_toggle_post_down_vote(const char* post_id, const char* user_id)
{
  return sql_procedure(TOGGLE_POST_DOWN_VOTE, post_id, user_id);
}


int sql_toggle_comment_up_vote(const char* comment_id, const char* user_id)
{
  return sql_procedure(TOGGLE_COMMENT_UP_VOTE, comment_id, user_id);
}


int sql_toggle_comment_down_vote(const char* comment_id, const char* user_id)
{
  return sql_procedure(TOGGLE_COMMENT_DOWN_VOTE, comment_id, user_id);
}


char* sql_create_user(const char* user_name, const char* passwd_hash, const char* about)
{
  return sql_function(CREATE_USER, user_name, passwd_hash, about);
}


char* sql_create_comment(const char* user_id, const char* post_id, const char* community_id, const char* body)
{
  return sql_function(CREATE_COMMENT, user_id, post_id, community_id, body);
}


char* sql_create_post(const char* user_id, const char* community_id, const char* title, const char* body)
{
  return sql_function(CREATE_POST, user_id, community_id, title, body);
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
