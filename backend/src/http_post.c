#define _XOPEN_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <kylestructs.h>

#include <log_manager.h>
#include <auth_manager.h>
#include <sql_manager.h>
#include <string_map.h>
#include <response.h>
#include <server.h>
#include <get_form.h>
#include <http_get.h>
#include <http_post.h>


struct response* http_post(struct request* req)
{
  // should never be NULL
  if (req == NULL)
  {
    log_crit("http_post(): NULL request object");
  }

  // figure out which endpoint
  if (strcmp(req->uri, "./signup") == 0)
  {
    const char* uname = get_map_value_str(req->content, "uname");
    const char* passwd = get_map_value_str(req->content, "passwd");
    const char* about = get_map_value_str(req->content, "about");

    return post_signup(uname, passwd, about);
  }

  else if (strcmp(req->uri, "./login") == 0)
  {
    const char* uname = get_map_value_str(req->content, "uname");
    const char* passwd = get_map_value_str(req->content, "passwd");

    return post_login(uname, passwd);
  }

  else if (strcmp(req->uri, "./logout") == 0)
  {
    if (req->client_info)
    {
      remove_token(req->client_info->token);
    }

    return response_redirect("/home");
  }

  else if (strcmp(req->uri, "./vote") == 0)
  {
    const char* type = get_map_value_str(req->content, "type");
    const char* direction = get_map_value_str(req->content, "direction");
    const char* id = get_map_value_str(req->content, "id");

    return post_vote(type, direction, id, req->client_info);
  }

  return response_error(STAT404);
}


static struct response* post_login(const char* uname, const char* passwd)
{
  const char* token;
  if (uname == NULL || passwd == NULL || (token = login_user(uname, passwd)) == NULL)
  {
    return get_login(NULL, LOGINERR_BAD_LOGIN);
  }

  // build redirect
  struct response* resp = response_redirect("/home");

  // add token to response
  char token_hdr[128];
  sprintf(token_hdr, COOKIE_TEMPLATE, token);
  ks_list_add(resp->header, ks_datacont_new(token_hdr, KS_CHARP, strlen(token_hdr)));

  return resp;
}


static struct response* post_signup(const char* uname, const char* passwd, const char* about)
{
  const char* token;
  if (uname == NULL || passwd == NULL)
  {
    log_info("Signup failed: null username or password");
    return get_login(NULL, LOGINERR_EMPTY);
  }

  if ((token = new_user(uname, passwd, about)) == NULL)
  {
    log_info("Signup failed: username already exists");
    return get_login(NULL, LOGINERR_UNAME_TAKEN);
  }

  // build redirect
  struct response* resp = response_redirect("/home");

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
    return response_redirect("/login");
  }

  if (type == NULL || direction == NULL || id == NULL)
  {
    return response_error(STAT400);
  }

  if (strcmp(type, "post") == 0)
  {
    if (strcmp(direction, "up") == 0)
    {
      sql_toggle_post_up_vote(id, client_info->user_id);
    }
    else if (strcmp(direction, "down") == 0)
    {
      sql_toggle_post_down_vote(id, client_info->user_id);
    }
    else return response_error(STAT400);
  }
  else if (strcmp(type, "comment") == 0)
  {
    if (strcmp(direction, "up") == 0)
    {
      sql_toggle_comment_up_vote(id, client_info->user_id);
    }
    else if (strcmp(direction, "down") == 0)
    {
      sql_toggle_comment_down_vote(id, client_info->user_id);
    }
    else return response_error(STAT400);
  }

  // prepare 200 response object
  struct response* resp = calloc(1, sizeof(struct response));
  resp->header = ks_list_new();
  ks_list_add(resp->header, ks_datacont_new(STAT200, KS_CHARP, strlen(STAT200)));

  char* contlen = "Content-Length: 0\r\n";
  ks_list_add(resp->header, ks_datacont_new(contlen, KS_CHARP, strlen(contlen)));

  return resp;
}
