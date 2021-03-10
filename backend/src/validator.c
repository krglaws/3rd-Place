#include <string.h>
#include <ctype.h>
#include <kylestructs.h>

#include <sql_manager.h>
#include <validator.h>


// used for converting percent encoded chars to ASCII
static short get_hex_char_val(char c);
static char hex_to_char(const char* hex);
static int url_decode(char* dest, const char* src);

static enum validation_result valid_content(char* dest, const char* src, int min_len, int max_len);
static enum validation_result valid_name(const char* name, int min_len, int max_len);

// encodes content into HTML-friendly form
static void html_encode(char* dest, char* src);


static short get_hex_char_val(char c)
{
  if (!isxdigit(c))
  {
    return -1;
  }

  if (isdigit(c))
  {
    return c - '0';
  }

  if (isupper(c))
  {
    return 10 + (c - 'A');
  }

  if (islower(c))
  {
    return 10 + (c - 'a');
  }

  return -1;
}


static char hex_to_char(const char* hex)
{
  short c1;
  if ((c1 = get_hex_char_val(*hex)) == -1)
  {
    return '\0';
  }

  short c2;
  if ((c2 = get_hex_char_val(*(hex+1))) == -1)
  {
    return '\0';
  }

  return (((char) c1) << 4) + ((char)c2);
}


/* Returns the length of the decoded string,
   -1 if an invalid encoding was found. */
static int url_decode(char* dest, const char* src)
{
  if (src == NULL)
  {
    return 0;
  }

  char c;
  int i = 0, j = 0;
  while (src[i] != '\0')
  {
    if (src[i] == '%' && src[i + 1] != '\0' && src[i + 2] != '\0')
    {
      if ((c = hex_to_char(src+i+1)) == '\0')
      {
        // invalid encoding
        return -1;
      }
      dest[j++] = c; 
      i += 3;
    }
    else if (src[i] == '+')
    {
      dest[j++] = ' ';
      i++;
    }
    else if(isalnum(src[i]) || src[i] == '*' || src[i] == '-' || src[i] == '_')
    {
      dest[j++] = src[i++];
    }
    else
    {
      // invalid char
      return -1;
    }
  }

  dest[j] = '\0';

  return j;
}


static void html_encode(char* dest, char* src)
{
  while (*src != '\0')
  {
    switch (*src)
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


enum validation_result valid_passwd(char* dest, const char* src)
{
  // caller must allocate buffer for
  // decoded password string
  int len;
  if ((len = url_decode(dest, src)) == -1)
  {
    return VALRES_INV_ENC;
  }

  if (len < MIN_PASSWD_LEN)
  {
    return VALRES_TOO_SHORT;
  }

  // at least 1 upper, 1 lower, 1 digit
  char c;
  int upper = 0, lower = 0, number = 0;
  for (int i = 0; i < len; i++)
  {
    c = dest[i];

    if (len > MAX_PASSWD_LEN)
    {
      return VALRES_TOO_LONG;
    }

    if (isupper(c))
    {
      upper++;
    }

    if (islower(c))
    {
      lower++;
    }

    if (isdigit(c))
    {
      number++;
    }
  }

  if (!(upper && lower && number))
  {
    return VALRES_UNMET;
  }

  return VALRES_OK;
}


static enum validation_result valid_name(const char* name, int min_len, int max_len)
{
  if (name == NULL && min_len > 0)
  {
    return VALRES_TOO_SHORT;
  }

  // no need to decode since valid name strings
  // do not contain chars that would be encoded
  char c;
  int len = 0;
  while ((c = name[len++]) != '\0')
  {
    if (len > max_len)
    {
      return VALRES_TOO_LONG;
    }

    if (!(isalnum(c) || c == '_'))
    {
      return VALRES_INV_CHAR;
    }
  }

  if (len < min_len)
  {
    return VALRES_TOO_SHORT;
  }

  return VALRES_OK;
}


enum validation_result valid_user_name(const char* user_name)
{
  return valid_name(user_name, MIN_USER_NAME_LEN, MAX_USER_NAME_LEN);
}


enum validation_result valid_community_name(const char* community_name)
{
  return valid_name(community_name, MIN_COMMUNITY_NAME_LEN, MAX_COMMUNITY_NAME_LEN);
}


static enum validation_result valid_content(char* dest, const char* src, int min_len, int max_len)
{
  // decoded string will always be <= src length
  int len;
  char tmp[max_len+1];
  if ((len = url_decode(tmp, src)) == -1)
  {
    return VALRES_INV_ENC;
  }

  if (len > max_len)
  {
    return VALRES_TOO_LONG;
  }

  if (len < min_len)
  {
    return VALRES_TOO_SHORT;
  }

  html_encode(dest, tmp);

  return VALRES_OK;
}


enum validation_result valid_user_about(char* dest, const char* src)
{
  return valid_content(dest, src, MIN_USER_ABOUT_LEN, MAX_USER_ABOUT_LEN);
}


enum validation_result valid_community_about(char* dest, const char* src)
{
  return valid_content(dest, src, MIN_COMMUNITY_ABOUT_LEN, MAX_COMMUNITY_ABOUT_LEN);
}


enum validation_result valid_comment_body(char* dest, const char* src)
{
  return valid_content(dest, src, MIN_COMMENT_BODY_LEN, MAX_COMMENT_BODY_LEN);
}


enum validation_result valid_post_body(char* dest, const char* src)
{
  return valid_content(dest, src, MIN_POST_BODY_LEN, MAX_POST_BODY_LEN);
}


enum validation_result valid_post_title(char* dest, const char* src)
{
  return valid_content(dest, src, MIN_POST_TITLE_LEN, MAX_POST_TITLE_LEN);
}
