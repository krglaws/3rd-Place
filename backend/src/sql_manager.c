
#include <stdio.h>
#include <stdlib.h>
#include <my_global.h>
#include <mysql.h>

#include <sql_manager.h>


static MYSQL* sqlcon;


void init_sql_manager()
{
  if ((sqlcon = mysql_init(NULL)) == NULL)
  {
    fprintf(stderr, "%s\n", mysql_error(NULL));
    exit(EXIT_FAILURE);
  }

  if (mysql_real_connect(sqlcon, "localhost", "falcon", "falcon123",
			  "falcondb", 3306, NULL, 0) == NULL)
  {
    fprintf(stderr, "%s\n", mysql_error(NULL));
    mysql_close(sqlcon);
    exit(EXIT_FAILURE);
  }
}


void terminate_sql_manager()
{
  mysql_close(sqlcon);
}


char*** query_database(char* query)
{
  MYSQL_RES* result;
  MYSQL_ROW sqlrow;

  if (query == NULL)
  {
    fprintf(stderr, "query_database(): null query\n");
    return NULL;
  }

  if (mysql_query(sqlcon, query))
  {
    fprintf(stderr, "%s\n", mysql_error(sqlcon));
    return NULL;
  }

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

