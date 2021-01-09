#include <kylestructs.h>
#include <string.h>

#include <string_map.h>


ks_hashmap* string_to_map(char* str, const char* delim1, const char* delim2)
{
  ks_hashmap* map = ks_hashmap_new(KS_CHARP, 8);
  ks_list* line_list = ks_list_new();

  char* token;
  if ((token = strtok(str, delim1)) == NULL)
  {
    ks_list_delete(line_list);
    return map;
  }

  if (ks_list_add(line_list, ks_datacont_new(token, KS_CHARP, strlen(token))) == -1)
  {
    ks_list_delete(line_list);
    return map;
  }

  while (1)
  {
    if ((token = strtok(NULL, delim1)) == NULL)
    {
      break;
    }

    ks_list_add(line_list, ks_datacont_new(token, KS_CHARP, strlen(token)));
  }

  ks_datacont* key, *val;
  int num_lines = ks_list_length(line_list);

  for (int i = 0; i < num_lines; i++)
  {
    if ((token = strtok(ks_list_get(line_list, i)->cp, delim2)) == NULL ||
          (key = ks_datacont_new(token, KS_CHARP, strlen(token))) == NULL)
    {
      ks_list_delete(line_list);
      return map;
    }

    if ((token = strtok(NULL, delim2)) == NULL ||
          (val = ks_datacont_new(token, KS_CHARP, strlen(token))) == NULL)
    {
      ks_datacont_delete(key);
      ks_list_delete(line_list);
      return map;
    }

    if (ks_hashmap_add(map, key, val) == -1)
    {
      ks_datacont_delete(key);
      ks_datacont_delete(val);
      ks_list_delete(line_list);

      return map;
    }
  }

  ks_list_delete(line_list);

  return map;
}


const char* get_map_val(const ks_hashmap* map, const char* keystr)
{
  ks_datacont* key = ks_datacont_new(keystr, KS_CHARP, strlen(keystr));

  const ks_datacont* val = ks_hashmap_get(map, key);
  ks_datacont_delete(key);

  return val->cp;
}
