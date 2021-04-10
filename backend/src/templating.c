#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <kylestructs.h>

#include <log_manager.h>
#include <file_manager.h>
#include <string_map.h>
#include <http_get.h>
#include <templating.h>


static int tmplt_beg_len = sizeof(TEMPLATE_BEGIN)-1;
static int tmplt_end_len = sizeof(TEMPLATE_END)-1;


#define COPY_INTO_TEMPLATE() do\
{\
  int new_len = (val_len + tmplt_len) - (key_len + tmplt_beg_len + tmplt_end_len);\
  int beg_len = next - tmplt;\
  char* new_tmplt = malloc((new_len + 1) * sizeof(char));\
\
  memcpy(new_tmplt, tmplt, beg_len);\
  memcpy(new_tmplt + beg_len, val_str, val_len);\
  memcpy(new_tmplt + beg_len + val_len,\
         tmplt + beg_len + key_len + tmplt_beg_len + tmplt_end_len,\
         (int)(new_len - (beg_len + val_len)));\
  new_tmplt[new_len] = '\0';\
\
  free(tmplt);\
  tmplt = new_tmplt;\
  tmplt_len = new_len;\
  next = tmplt + beg_len + val_len;\
} while(0)


char* build_template(const ks_hashmap* page_data)
{
  if (page_data == NULL)
  {
    log_err("build_template(): page data is NULL");
    return NULL;
  }

  const ks_datacont* tmplt_path;
  if ((tmplt_path = get_map_value(page_data, TEMPLATE_PATH_KEY)) == NULL)
  {
    log_err("build_template(): page data is missing template path");
    return NULL;
  }

  struct file_pkg* pkg;
  if ((pkg = load_file(tmplt_path->cp)) == NULL)
  {
    log_err("build_template(): failed to load template file: '%s'", tmplt_path->cp);
    return NULL;
  }
  char* tmplt = pkg->contents;
  int tmplt_len = pkg->length;
  free(pkg);

  char* next = tmplt;

  while ((next = strstr(tmplt, TEMPLATE_BEGIN)) != NULL)
  {
    // find key length
    char* end = strstr(next + tmplt_beg_len, TEMPLATE_END);
    char* next_next;
    if (end == NULL || (((next_next = strstr(next + tmplt_beg_len, TEMPLATE_BEGIN)) != NULL) && next_next < end))
    {
      log_err("build_template(): TEMPLATE_BEGIN string is missing a corresponding TEMPLATE_END in file: '%s'", tmplt_path->cp);
      free(tmplt);
      return NULL;
    }
    int key_len = end - (next + tmplt_beg_len);

    // read key into buffer
    char key[key_len + 1];
    memcpy(key, next + tmplt_beg_len, key_len);
    key[key_len] = '\0';

    // get value from map
    const ks_datacont* val_dc = get_map_value(page_data, key);

    if (val_dc == NULL)
    {
      if (strstr(key, OPTIONAL_PREFIX) != key)
      {
        log_err("build_template(): missing page data for key '%s' in file: '%s'\n",
                key,
                tmplt_path->cp);
        free(tmplt);
        return NULL;
      }

      char* val_str = "";
      int val_len = 0;
      COPY_INTO_TEMPLATE();

      next += (key_len + tmplt_beg_len + tmplt_end_len);
    }

    else if (val_dc->type == KS_HASHMAP)
    {
      char* val_str;
      if ((val_str = build_template(val_dc->hm)) == NULL)
      {
        free(tmplt);
        return NULL;
      }
      int val_len = strlen(val_str);

      COPY_INTO_TEMPLATE();

      free(val_str);
    }

    else if (val_dc->type == KS_LIST)
    {
      int val_len = 0;
      char* val_str = NULL;

      // iterate through list, combine into single string
      const ks_datacont* curr;
      ks_iterator* iter = ks_iterator_new(val_dc->ls, KS_LIST);
      while ((curr = ks_iterator_next(iter)) != NULL)
      {
        char* temp_str;
        if ((temp_str = build_template(curr->hm)) == NULL)
        {
          ks_iterator_delete(iter);
          free(tmplt);
          return NULL;
        }
        int temp_len = strlen(temp_str);

        val_str = realloc(val_str, (temp_len + val_len + 1) * sizeof(char));
        memcpy(val_str + val_len, temp_str, temp_len);
        free(temp_str);

        val_len = temp_len + val_len;
        val_str[val_len] = '\0';
      }
      ks_iterator_delete(iter);

      COPY_INTO_TEMPLATE();

      free(val_str);
    }

    else if (val_dc->type == KS_CHARP)
    {
      char* val_str = val_dc->cp;
      int val_len = strlen(val_str);

      COPY_INTO_TEMPLATE();
    }

    else
    {
      log_crit("build_template(): Invalid ks_datacont type found in page data");
    }
  }

  return tmplt;
}
