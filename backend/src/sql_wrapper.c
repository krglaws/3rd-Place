
#include <stdlib.h>
#include <sql_manager.h>
#include <kylestructs.h>

#include <sql_wrapper.h>


list** query_database_ls(char* query)
{
  char*** result = query_database(query);
  list** result_ls = malloc( sizeof(list*));

  int i;
  for (i = 0; result[i] != NULL; i++)
  {
    result_ls = realloc(result_ls, i + 2);
    result_ls[i] = list_new();
    for (int j = 0; result[i][j] != NULL; j++)
    {
      if (result[i][j] == NULL)
      {
        list_add(result_ls[i], datacont_new("NULL", CHARP, 4));
      }
      else
      {
        list_add(result_ls[i], datacont_new(result[i][j], CHARP, strlen(result[i][j])));
      }
      free(result[i][j]);
    }
    free(result[i]);
  }
  free(result);

  result_ls[i] = NULL;

  return result_ls;
}

