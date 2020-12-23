#define _XOPEN_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <kylestructs.h>

#include <log_manager.h>
#include <auth_manager.h>
#include <senderr.h>
#include <common.h>
#include <http_get.h>
#include <http_post.h>


struct response* http_post(struct request* req)
{
  if (req == NULL)
  {
    return NULL;
  }

  struct response* resp = NULL;

  // parse post arguments
  treemap* args = parse_args(req->content);

  // figure out which endpoint
  if (strcmp(req->uri, "./signup") == 0)
  {
    const char* uname = get_arg(args, "uname");
    const char* passwd = get_arg(args, "passwd");
    resp = post_signup(uname, passwd);
  }

  else if (strcmp(req->uri, "./login") == 0)
  {
    const char* uname = get_arg(args, "uname");
    const char* passwd = get_arg(args, "passwd");
    resp = post_login(uname, passwd);
  }

  else if (strcmp(req->uri, "./logout") == 0)
  {
    if (req->client_info)
    {
      remove_token(req->client_info->token);
    }

    resp = redirect("/home");
  }

  treemap_delete(args);

  if (resp == NULL)
  {
    return senderr(ERR_NOT_FOUND);
  }

  return resp;
}


static const char* get_arg(const treemap* args, const char* argname)
{
  datacont* key = datacont_new(argname, CHARP, strlen(argname));
  datacont* val = treemap_get(args, key);

  const char* val_str = val ? val->cp : NULL;

  datacont_delete(key);

  return val_str;
}


static int add_key_val(treemap* map, char* token)
{
  char* delim = "=";
  datacont *key, *val;

  if ((token = strtok(token, delim)) == NULL ||
      (key = datacont_new(token, CHARP, strlen(token))) == NULL)
  {
    return -1;
  }

  if ((token = strtok(NULL, delim)) == NULL ||
      (val = datacont_new(token, CHARP, strlen(token))) == NULL)
  {
    datacont_delete(key);
    return -1;
  }

  if ((treemap_add(map, key, val)) == -1)
  {
    datacont_delete(key);
    datacont_delete(val);
    return -1;
  }

  return 0;
}


static treemap* parse_args(char* str)
{
  treemap* map = treemap_new();

  char* delim = "&";
  char* token = strtok(str, delim);
  int len = strlen(str);
  int currlen = 0;

  while (token != NULL)
  {
    currlen += strlen(token) + 1;

    // end of args reached
    if (currlen >= len)
    {
      currlen -= 1;
    }

    if (add_key_val(map, token) == -1)
    {
      treemap_delete(map);
      return NULL;
    }

    token = strtok(str + len, delim);
  }

  return map;
}


static struct response* bad_login_signup(enum login_error e)
{
  // use http_get's get_login()
  char* content = get_login(NULL, e);

  // prepare response object
  struct response* resp = calloc(1, sizeof(struct response));
  resp->header = list_new();
  list_add(resp->header, datacont_new(STAT200, CHARP, strlen(STAT200)));
  list_add(resp->header, datacont_new(TEXTHTML, CHARP, strlen(TEXTHTML)));

  // build content-length header line
  int contlen = strlen(content);
  char contlenline[80];
  int len = sprintf(contlenline, "Content-Length: %d\n", contlen);

  // add to response object
  list_add(resp->header, datacont_new(contlenline, CHARP, len));
  resp->content = content;
  resp->content_length = contlen;

  return resp;
}


static struct response* redirect(const char* uri)
{
  if (uri == NULL)
  {
    log_crit("redirect(): null redirect uri");
  }

  char redirect_hdr[64];
  int len = sprintf(redirect_hdr, REDIRECT_TEMPLATE, uri);

  struct response* resp = calloc(1, sizeof(struct response));
  resp->header = list_new();
  list_add(resp->header, datacont_new(STAT302, CHARP, strlen(STAT302)));
  list_add(resp->header, datacont_new(redirect_hdr, CHARP, len));

  return resp;
}


static struct response* post_login(const char* uname, const char* passwd)
{
  const char* token;
  if (uname == NULL || passwd == NULL || (token = login_user(uname, passwd)) == NULL)
  {
    return bad_login_signup(LOGINERR_BAD_LOGIN);
  }

  // build redirect
  struct response* resp = redirect("/home");

  // add token to response
  char token_hdr[128];
  sprintf(token_hdr, COOKIE_TEMPLATE, token);
  list_add(resp->header, datacont_new(token_hdr, CHARP, strlen(token_hdr)));

  return resp;
}


static struct response* post_signup(const char* uname, const char* passwd)
{
  const char* token;
  if (uname == NULL || passwd == NULL)
  {
    return bad_login_signup(LOGINERR_EMPTY);
  }

  if ((token = new_user(uname, passwd)) == NULL)
  {
    return bad_login_signup(LOGINERR_UNAME_TAKEN);
  }

  // build redirect
  struct response* resp = redirect("/home");

  // add token to response
  char token_hdr[128];
  sprintf(token_hdr, COOKIE_TEMPLATE, token);
  list_add(resp->header, datacont_new(token_hdr, CHARP, strlen(token_hdr)));

  return resp;
}


