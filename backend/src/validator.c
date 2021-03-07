#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <kylestructs.h>

#include <load_file.h>
#include <string_map.h>
#include <log_manager.h>
#include <sql_manager.h>


static ks_hashmap* url_code_map = NULL;
static ks_hashmap* html_code_map = NULL;


int init_validator()
{
  const char* url_code_path = "backend/url_encoding.txt";
  char* url_code_file;
  if ((url_code_file = load_file(url_code_path)) == NULL)
  {
    log_crit("init_validator(): failed to open file '%s'", url_code_path);
  }

  url_code_map = string_to_map(url_code_file, "\n", ": ");
  free(url_code_file);

  const char* html_code_path = "backend/html_encoding.txt";
  char* html_code_file;
  if ((html_code_file = load_file(html_code_path)) == NULL)
  {
    ks_hashmap_delete(url_code_map);
    log_crit("init_validator(): failed to open file '%s'", html_code_path);
  }

  html_code_map = string_to_map(html_code_file, "\n", ": ");
  free(html_code_file);
}


static int url_decode(char* dest, const char* src)
{
  const char* c;
  char key[4];

  while (src[0] != '\0')
  {
    if (src[0] == '%' && src[1] != '\0' && src[2] == '\0')
    {
      memcpy(key, src, 3);
      if ((c = get_map_value_str(url_code_map, key)) == NULL)
      {
        // invalid encoding
        return -1;
      }

      *dest = *c;
      dest++;
      src += 3;
    }
    else if (src[0] == '+')
    {
      *dest = ' ';
      dest++;
      src++;
    }
    else if(isalnum(src[0]) || src[0] == '*' || src[0] == '-' || src[0] == '_')
    {
      *dest = *src;
      dest++;
      src++;
    }
    else
    {
      // invalid char
      return -1;
    }
  }

  *dest = '\0';

  return 0;
}


static void html_encode(char* dest, char* src)
{
  while (src[0] != '\0')
  {
    switch (src[0])
    {
      case '&':
        memcpy(dest, "&amp;", 5);
        dest += 5;
        break;
      case '<':
        memcpy(dest, "&lt;", 4);
        dest += 4;
        break;
      case '>':
        memcpy(dest, "&gt;", 4);
        dest += 4;
        break;
      default:
        *dest = *src;
        dest++;
    }
    src++;
  }

  *dest = '\0';
}


bool valid_name(const char* name, int max_len)
{
  int len = strlen(name);

  if (!(len < max_len))
  {
    return false;
  }

  for (int i = 0; i < len; i++)
  {
    char c = name[i];
    if (!(isalnum(c) || c == '_'))
    {
      return false;
    }
  }

  return true;
}


bool valid_content(char* dest, const char* src, int max_len)
{
  char tmp[max_len];
  if (url_decode(tmp, src))
  {
    return false;
  }

  if (!(strlen(tmp) < max_len))
  {
    return false;
  }

  html_encode(dest, tmp);
}
