#define _XOPEN_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <kylestructs.h>

#include <log_manager.h>
#include <auth_manager.h>
#include <senderr.h>
#include <common.h>
#include <string_map.h>
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
  ks_hashmap* args = string_to_map(req->content, "&", "=");

  // figure out which endpoint
  if (strcmp(req->uri, "./signup") == 0)
  {
    const char* uname = get_map_val(args, "uname");
    const char* passwd = get_map_val(args, "passwd");
    resp = post_signup(uname, passwd);
  }

  else if (strcmp(req->uri, "./login") == 0)
  {
    const char* uname = get_map_val(args, "uname");
    const char* passwd = get_map_val(args, "passwd");
    resp = post_login(uname, passwd);
  }

  else if (strcmp(req->uri, "./logout") == 0)
  {
    if (req->client_info)
    {
      log_info("User '%s' logged out", req->client_info->user_name);
      remove_token(req->client_info->token);
    }

    resp = redirect("/home");
  }

  ks_hashmap_delete(args);

  if (resp == NULL)
  {
    return senderr(ERR_NOT_FOUND);
  }

  return resp;
}


static struct response* bad_login_signup(enum login_error e)
{
  // use http_get's get_login()
  char* content = get_login(NULL, e);

  // prepare response object
  struct response* resp = calloc(1, sizeof(struct response));
  resp->header = ks_list_new();
  ks_list_add(resp->header, ks_datacont_new(STAT200, KS_CHARP, strlen(STAT200)));
  ks_list_add(resp->header, ks_datacont_new(TEXTHTML, KS_CHARP, strlen(TEXTHTML)));

  // build content-length header line
  int contlen = strlen(content);
  char contlenline[80];
  int len = sprintf(contlenline, "Content-Length: %d\n", contlen);

  // add to response object
  ks_list_add(resp->header, ks_datacont_new(contlenline, KS_CHARP, len));
  resp->content = content;
  resp->content_length = contlen;

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
  ks_list_add(resp->header, ks_datacont_new(token_hdr, KS_CHARP, strlen(token_hdr)));

  return resp;
}


static struct response* post_signup(const char* uname, const char* passwd)
{
  const char* token;
  if (uname == NULL || passwd == NULL)
  {
    log_info("Signup failed: null username or password");
    return bad_login_signup(LOGINERR_EMPTY);
  }

  if ((token = new_user(uname, passwd)) == NULL)
  {
    log_info("Signup failed: username already exists");
    return bad_login_signup(LOGINERR_UNAME_TAKEN);
  }

  // build redirect
  struct response* resp = redirect("/home");

  // add token to response
  char token_hdr[128];
  sprintf(token_hdr, COOKIE_TEMPLATE, token);
  ks_list_add(resp->header, ks_datacont_new(token_hdr, KS_CHARP, strlen(token_hdr)));

  return resp;
}
