#define _XOPEN_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <kylestructs.h>

#include <log_manager.h>
#include <auth_manager.h>
#include <sql_manager.h>
#include <senderr.h>
#include <common.h>
#include <string_map.h>
#include <http_get.h>
#include <http_post.h>


struct response* http_post(struct request* req)
{
  // should never be NULL
  if (req == NULL)
  {
    log_crit("http_post(): NULL request object");
  }

  struct response* resp = NULL;

  // figure out which endpoint
  if (strcmp(req->uri, "./signup") == 0)
  {
    const ks_datacont* uname = get_map_value(req->content, "uname");
    const ks_datacont* passwd = get_map_value(req->content, "passwd");
    resp = post_signup(uname ? uname->cp : NULL,
                       passwd ? passwd->cp : NULL);
  }

  else if (strcmp(req->uri, "./login") == 0)
  {
    const ks_datacont* uname = get_map_value(req->content, "uname");
    const ks_datacont* passwd = get_map_value(req->content, "passwd");
    resp = post_login(uname ? uname->cp : NULL,
                      passwd ? passwd->cp : NULL);
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

  else if (strcmp(req->uri, "./vote") == 0)
  {
    const ks_datacont* type = get_map_value(req->content, "type");
    const ks_datacont* direction = get_map_value(req->content, "direction");
    const ks_datacont* id = get_map_value(req->content, "id");

    resp = post_vote(type ? type->cp : NULL,
                     direction ? direction->cp : NULL,
                     id ? id->cp : NULL,
                     req->client_info);
  }

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
  int len = sprintf(contlenline, "Content-Length: %d\r\n", contlen);

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


static struct response* post_vote(const char* type, const char* direction, const char* id, const struct auth_token* client_info)
{
  if (client_info == NULL)
  {
    return redirect("/login");
  }

  if (type == NULL || direction == NULL || id == NULL)
  {
    return senderr(ERR_BAD_REQ);
  }

  if (strcmp(type, "post") == 0)
  {
    if (strcmp(direction, "up") == 0)
    {
      toggle_post_upvote(id, client_info->user_id);
    }
    else if (strcmp(direction, "down") == 0)
    {
      toggle_post_downvote(id, client_info->user_id);
    }
    else return senderr(ERR_BAD_REQ);
  }
  else if (strcmp(type, "comment") == 0)
  {
    if (strcmp(direction, "up") == 0)
    {
      toggle_comment_upvote(id, client_info->user_id);
    }
    else if (strcmp(direction, "down") == 0)
    {
      toggle_comment_downvote(id, client_info->user_id);
    }
    else return senderr(ERR_BAD_REQ);
  }

  // prepare 200 response object
  struct response* resp = calloc(1, sizeof(struct response));
  resp->header = ks_list_new();
  ks_list_add(resp->header, ks_datacont_new(STAT200, KS_CHARP, strlen(STAT200)));

  char* contlen = "Content-Length: 0\r\n";
  ks_list_add(resp->header, ks_datacont_new(contlen, KS_CHARP, strlen(contlen)));

  return resp;
}
