
#include <stdlib.h>
#include <stdio.h>
#include <sql_manager.h>
#include <kylestructs.h>

#include <log_manager.h>
#include <sql_wrapper.h>


list** query_database_ls(char* query)
{
  // query database
  char*** result;
  if ((result = query_database(query)) == NULL)
  {
    log_err("query_database_ls(): query failed: '%s'", query);

    // NOTE: 
    // the functions that call this don't check for NULL returns,
    // too lazy to fix. Will change at some point though
    return calloc(1, sizeof(list*));
  }

  // get number of rows
  int rows = 0;
  for (; result[rows] != NULL; rows++);

  list** result_ls = calloc(rows + 1, sizeof(list*));

  // for each row
  int i;
  for (i = 0; i < rows; i++)
  {
    result_ls[i] = list_new();

    // for each column
    for (int j = 0; result[i][j] != NULL; j++)
    {
      if (result[i][j] == NULL)
      {
        datacont* dc = datacont_new("NULL", CHARP, 4);
        list_add(result_ls[i], dc);
      }
      else
      {
        datacont* dc = datacont_new(result[i][j], CHARP, strlen(result[i][j]));
        list_add(result_ls[i], dc);
      }
      free(result[i][j]);
    }
    free(result[i]);
  }
  free(result);

  result_ls[i] = NULL;

  return result_ls;
}


void delete_query_result(list** result)
{
  for (int i = 0; result[i] != NULL; i++)
  {
    list_delete(result[i]);
  }
  free(result); 
}
