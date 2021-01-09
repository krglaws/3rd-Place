
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql.h>

#include <log_manager.h>
#include <util.h>
#include <string_map.h>
#include <sql_manager.h>


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

  const char* dbhost = get_map_val(args, "DBHOST");
  const char* dbname = get_map_val(args, "DBNAME");
  const char* dbuser = get_map_val(args, "DBUSER");
  const char* dbpass = get_map_val(args, "DBPASS");

  // initialize sql connection object
  if ((sqlcon = mysql_init(NULL)) == NULL)
  {
    ks_hashmap_delete(args);
    log_crit("init_sql_manager(): mysql_init(): %s", mysql_error(NULL));
  }

  // connect to mysql database
  if (mysql_real_connect(sqlcon, dbhost, dbuser, dbpass,
      dbname, 0, NULL, 0) == NULL)
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


char*** query_database(char* query)
{
  /* check for null query string */
  if (query == NULL)
  {
    log_crit("query_database(): NULL query");
  }

  /* submit query */
  if (mysql_query(sqlcon, query))
  {
    log_crit("query_database(): mysql_query(): %s", mysql_error(sqlcon));
  }

  /* store result */
  MYSQL_RES* result = mysql_store_result(sqlcon);

  if (mysql_errno(sqlcon) != 0)
  {
    log_crit("query_database(): mysql_store_result(): %s", mysql_error(sqlcon));
  }

  if (result == NULL)
  {
    mysql_free_result(result);
    return NULL;
  }

  int num_rows = mysql_num_rows(result);
  char*** rows = calloc(num_rows + 1, sizeof(char**));

  MYSQL_ROW sqlrow;
  for (int i = 0; i < num_rows; i++)
  {
    sqlrow = mysql_fetch_row(result);
    int num_fields = mysql_num_fields(result);
    rows[i] = calloc(num_fields + 1, sizeof(char*));
    for (int j = 0; j < num_fields; j++)
    {
      if (sqlrow[j])
      {
        int fieldlen = strlen(sqlrow[j]);
        rows[i][j] = calloc((fieldlen + 1), sizeof(char));
        memcpy(rows[i][j], sqlrow[j], fieldlen);
      }
      else
      {
        char* nullstr = "NULL";
        rows[i][j] = calloc(5, sizeof(char));
        memcpy(rows[i][j], nullstr, 5);
      }
    }
  }

  mysql_free_result(result);

  return rows;
}

