
#include <stdio.h>
#include <my_global.h>
#include <mysql.h>
#include <kylestructs.h>

#include "include/sql_manager.h"


MYSQL* sqlcon;


void init_sql_manager()
{
  if ((sqlcon = mysql_init(NULL)) == NULL)
  {
    fprintf(stderr, "%s", mysql_error(NULL));
    exit(EXIT_FAILURE);
  }

  if (mysql_real_connect(sqlcon, "localhost", "falcon", "falcon123",
			  "falcondb", 3306, NULL, 0) == NULL)
  {
    fprintf(stderr, "%s", mysql_error(NULL));
    mysql_close(sqlcon);
    exit(EXIT_FAILURE);
  }
}


list** query_database(char* query)
{
  MYSQL_RES* result;
  MYSQL_ROW sqlrow;
  list** rows;

  if (query == NULL) return NULL;

  if (mysql_query(sqlcon, query))
  {
    fprintf(stderr, mysql_error(sqlcon));
    return NULL;
  }

  result = mysql_store_result(sqlcon);
  if (result == NULL)
  {
    fprintf(stderr, mysql_error(sqlcon));
    return NULL;
  }

  int num_rows = mysql_num_rows(result);
  rows = calloc(sizeof(list*) * (num_rows + 1));

  for (int i = 0; i < num_rows; i++)
  {
    rows[i] = list_new();
    sqlrow = mysql_fetch_row(result);
    int num_fields = mysql_num_fields(result);
    for (int j = 0; j < num_fields; j++)
    {
      if (sqlrow[j]) list_add(rows[i], datacont_new(sqlrow[j], CHARP, strlen(sqlrow[j])));
      else list_add(rows[i], datacont_new("NULL", CHARP, 4));
    }
  }

  mysql_free_result(result);

  return rows;
}

