#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sql_manager.h>
#include <kylestructs.h>

#include <log_manager.h>
#include <sql_wrapper.h>


ks_list** query_database_ls(char* query)
{
  // query database
  char*** result;
  if ((result = query_database(query)) == NULL)
  {
    // NOTE: 
    // the functions that call this don't check for NULL returns,
    // too lazy to fix. Will change at some point though
    return calloc(1, sizeof(ks_list*));
  }

  // get number of rows
  int rows = 0;
  for (; result[rows] != NULL; rows++);

  ks_list** result_ls = calloc(rows + 1, sizeof(ks_list*));

  // for each row
  int i;
  for (i = 0; i < rows; i++)
  {
    result_ls[i] = ks_list_new();

    // for each column
    for (int j = 0; result[i][j] != NULL; j++)
    {
      if (result[i][j] == NULL)
      {
        ks_datacont* dc = ks_datacont_new("NULL", KS_CHARP, 4);
        ks_list_add(result_ls[i], dc);
      }
      else
      {
        ks_datacont* dc = ks_datacont_new(result[i][j], KS_CHARP, strlen(result[i][j]));
        ks_list_add(result_ls[i], dc);
      }
      free(result[i][j]);
    }
    free(result[i]);
  }
  free(result);

  result_ls[i] = NULL;

  return result_ls;
}


void delete_query_result(ks_list** result)
{
  for (int i = 0; result[i] != NULL; i++)
  {
    ks_list_delete(result[i]);
  }
  free(result); 
}
