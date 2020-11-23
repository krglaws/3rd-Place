
#include <stdio.h>
#include <stdlib.h>
#include <my_global.h>
#include <mysql.h>

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
    fprintf(stderr, "get_config_value(): failed to find key '%s'\n", key);
    return -1;
  }

  // get length of value
  char* valbegin = keyloc + keylen + 1;
  int vallen = 0;
  while (*(valbegin + vallen) != '\n' && *(valbegin + vallen) != '\0') vallen++;

  // check for value length problems
  if (vallen == 0)
  {
    fprintf(stderr, "get_config_value(): empty value for key '%s'\n", key);
    return -1;
  }
  if (vallen > outlen)
  {
    fprintf(stderr, "get_config_value(): value for key '%s' too long\n", key);
    return -1;
  }

  // copy value into output buffer
  memcpy(outbuf, (keyloc + keylen + 1), vallen);
  outbuf[vallen] = '\0';

  return 0;
}


void init_sql_manager()
{

  // load db.config
  char* config;
  if ((config = load_file("backend/db.config")) == NULL)
  {
    fprintf(stderr, "init_sql_manager(): failed to load db.config\n");
    exit(EXIT_FAILURE);
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
    fprintf(stderr, "init_sql_manager(): failed to parse db.config\n");
    exit(EXIT_FAILURE);
  }
  free(config);

  // initialize sql connection object
  if ((sqlcon = mysql_init(NULL)) == NULL)
  {
    fprintf(stderr, "%s\n", mysql_error(NULL));
    exit(EXIT_FAILURE);
  }

  // connect to mysql database
  if (mysql_real_connect(sqlcon, dbhost, dbuser, dbpass,
      dbname, 0, NULL, 0) == NULL)
  {
    // this doesnt print anything for some reason
    //fprintf(stderr, "%s\n", mysql_error(NULL));
    fprintf(stderr, "Failed to connect to database\n");
    mysql_close(sqlcon);
    exit(EXIT_FAILURE);
  }
}


void terminate_sql_manager()
{
  mysql_close(sqlcon);
  mysql_server_end();
}


char*** query_database(char* query)
{
  MYSQL_RES* result;
  MYSQL_ROW sqlrow;

  /* check for null query string */
  if (query == NULL)
  {
    fprintf(stderr, "query_database(): null query\n");
    return NULL;
  }

  /* submit query */
  if (mysql_query(sqlcon, query))
  {
    fprintf(stderr, "%s\n", mysql_error(sqlcon));
    return NULL;
  }

  /* store result */
  result = mysql_store_result(sqlcon);
  if (result == NULL)
  {
    fprintf(stderr, "%s\n", mysql_error(sqlcon));
    return NULL;
  }

  int num_rows = mysql_num_rows(result);
  char*** rows = calloc(num_rows + 1, sizeof(char**));

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
        rows[i][j] = NULL;
      }
    }
  }

  mysql_free_result(result);

  return rows;
}

