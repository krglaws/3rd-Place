
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql.h>

#include <log_manager.h>
#include <util.h>
#include <sql_manager.h>


static MYSQL* sqlcon;


static int get_config_value(const char* config, const char* key, char* outbuf, int outlen)
{
  int keylen = strlen(key);
  char* keyloc = strstr(config, key);

  // check if key is present in config file
  if (keyloc == NULL)
  {
    log_err("get_config_value(): failed to find key '%s'", key);
    return -1;
  }

  // get length of value
  char* valbegin = keyloc + keylen + 1;
  int vallen = 0;
  while (*(valbegin + vallen) != '\n' && *(valbegin + vallen) != '\0') vallen++;

  // check for value length problems
  if (vallen == 0)
  {
    log_err("get_config_value(): empty value for key '%s'", key);
    return -1;
  }
  if (vallen > outlen)
  {
    log_err("get_config_value(): value for key '%s' too long", key);
    return -1;
  }

  // copy value into output buffer
  memcpy(outbuf, (keyloc + keylen + 1), vallen);
  outbuf[vallen] = '\0';

  return 0;
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

  char dbhost[32];
  char dbname[32];
  char dbuser[32];
  char dbpass[32];

  // load configuration values into buffers
  if (get_config_value(config, "DBHOST", dbhost, 32) ||
      get_config_value(config, "DBNAME", dbname, 32) ||
      get_config_value(config, "DBUSER", dbuser, 32) ||
      get_config_value(config, "DBPASS", dbpass, 32))
  {
    free(config);
    log_crit("init_sql_manager(): failed to parse db.config");
  }
  free(config);

  // initialize sql connection object
  if ((sqlcon = mysql_init(NULL)) == NULL)
  {
    log_crit("init_sql_manager(): mysql_init(): %s", mysql_error(NULL));
  }

  // connect to mysql database
  if (mysql_real_connect(sqlcon, dbhost, dbuser, dbpass,
      dbname, 0, NULL, 0) == NULL)
  {
    log_crit("init_sql_manager(): mysql_real_connect(): %s", mysql_error(sqlcon));
  }
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

