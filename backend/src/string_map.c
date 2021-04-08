#include <kylestructs.h>
#include <string.h>

#include <string_map.h>


ks_hashmap* string_to_map(char* str, const char* delim1, const char* delim2)
{
  ks_hashmap* map = ks_hashmap_new(KS_CHARP, 8);

  if (str == NULL)
  {
    return map;
  }

  ks_list* line_list = ks_list_new();

  char* tmp = str;
  while (*tmp != '\0')
  {
    char* end_of_line;
    if ((end_of_line = strstr(tmp, delim1)) == NULL)
    {
      end_of_line = tmp + strlen(tmp);
    }

    const ks_datacont* line_dc = ks_datacont_new(tmp, KS_CHARP, end_of_line - tmp);
    ks_list_add(line_list, line_dc);

    tmp = *(end_of_line) == '\0' ? 
          end_of_line : end_of_line + strlen(delim1);
  }

  const ks_datacont* line_dc;
  ks_iterator* iter = ks_iterator_new(line_list, KS_LIST);
  while ((line_dc = ks_iterator_next(iter)) != NULL)
  {
    char* end_of_key;
    if ((end_of_key = strstr(line_dc->cp, delim2)) == NULL)
    {
      end_of_key = line_dc->cp + line_dc->size;
    }

    int keylen = end_of_key - line_dc->cp;
    if (keylen == 0)
    {
      continue;
    }
    const ks_datacont* key = ks_datacont_new(line_dc->cp, KS_CHARP, keylen);

    int vallen = line_dc->size - (strlen(delim2) + keylen);
    const ks_datacont* val;
    if (vallen > 0)
    {
      end_of_key += strlen(delim2);
      val = ks_datacont_new(end_of_key, KS_CHARP, vallen);
    }
    else
    {
      val = ks_datacont_new("", KS_CHARP, 0);
    }

    ks_hashmap_add(map, key, val);
  }
  ks_iterator_delete(iter);
  ks_list_delete(line_list);

  return map;
}


const ks_datacont* get_map_value(const ks_hashmap* hm, const char* key)
{
  ks_datacont* key_dc = ks_datacont_new(key, KS_CHARP, strlen(key));
  const ks_datacont* val_dc = ks_hashmap_get(hm, key_dc);
  ks_datacont_delete(key_dc);

  return val_dc;
}


const char* get_map_value_str(const ks_hashmap* hm, const char* key)
{
  const ks_datacont* dc = get_map_value(hm, key);
  return dc != NULL ? dc->cp : NULL;
}


void add_map_value_str(ks_hashmap* hm, const char* key, const char* val)
{
  ks_datacont* key_dc = ks_datacont_new(key, KS_CHARP, strlen(key));
  ks_datacont* val_dc = ks_datacont_new(val, KS_CHARP, strlen(val));

  ks_hashmap_add(hm, key_dc, val_dc);
}


void add_map_value_vp(ks_hashmap* hm, const char* key, const void* val)
{
  ks_datacont* key_dc = ks_datacont_new(key, KS_CHARP, strlen(key));
  ks_datacont* val_dc = ks_datacont_new(val, KS_VOIDP, strlen(val));

  ks_hashmap_add(hm, key_dc, val_dc);
}


void add_map_value_ls(ks_hashmap* hm, const char* key, const ks_list* val)
{
  ks_datacont* key_dc = ks_datacont_new(key, KS_CHARP, strlen(key));
  ks_datacont* val_dc = ks_datacont_new(val, KS_LIST, 1);

  ks_hashmap_add(hm, key_dc, val_dc);
}


void add_map_value_hm(ks_hashmap* hm, const char* key, const ks_hashmap* val)
{
  ks_datacont* key_dc = ks_datacont_new(key, KS_CHARP, strlen(key));
  ks_datacont* val_dc = ks_datacont_new(val, KS_HASHMAP, 1);

  ks_hashmap_add(hm, key_dc, val_dc);
}
