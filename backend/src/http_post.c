#define _XOPEN_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <kylestructs.h>

#include <log_manager.h>
#include <auth_manager.h>
#include <sql_manager.h>
#include <validator.h>
#include <string_map.h>
#include <response.h>
#include <server.h>
#include <get_form.h>
#include <http_get.h>
#include <http_post.h>


static struct response* post_login(const char* uname, const char* passwd, const struct auth_token* client_info);
static struct response* post_signup(const char* uname, const char* passwd, const char* about, const struct auth_token* client_info);
static struct response* post_vote(const char* type, const char* direction, const char* id, const struct auth_token* client_info);
static struct response* post_comment(const char* post_id, const char* community_id, const char* body, const struct auth_token* client_info);


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

    return post_signup(uname, passwd, about, req->client_info);
  }

  else if (strcmp(req->uri, "./login") == 0)
  {
    const char* uname = get_map_value_str(req->content, "uname");
    const char* passwd = get_map_value_str(req->content, "passwd");

    return post_login(uname, passwd, req->client_info);
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

  else if (strcmp(req->uri, "./new_comment") == 0)
  {
    const char* post_id = get_map_value_str(req->content, "post_id");
    const char* community_id = get_map_value_str(req->content, "community_id");
    const char* body = get_map_value_str(req->content, "body");

    return post_comment(post_id, community_id, body, req->client_info);
  }

  else if (strcmp(req->uri, "./new_post") == 0)
  {
  }

  else if (strcmp(req->uri, "./new_community") == 0)
  {
  }

  return response_error(STAT404);
}


static struct response* post_login(const char* uname, const char* passwd, const struct auth_token* client_info)
{
  if (client_info != NULL)
  {
    // this will pull up the "you are already logged in" message
    return response_redirect("/login");
  }

  const char* token;
  char passwd_decoded[MAX_PASSWD_LEN + 1];

  if (uname == NULL || passwd == NULL ||
      valid_passwd(passwd_decoded, passwd) != VALRES_OK ||
      (token = login_user(uname, passwd_decoded)) == NULL)
  {
    // no need to indicate exactly why login failed
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


static struct response* post_signup(const char* uname, const char* passwd, const char* about, const struct auth_token* client_info)
{
  if (client_info != NULL)
  {
    // this will pull up the "you are already logged in" message
    return response_redirect("/login");
  }

  enum validation_result uname_valid = valid_user_name(uname);
  switch (uname_valid)
  {
    case VALRES_OK:
      break;
    case VALRES_TOO_SHORT:
      // SIGNUPERR_UNAME_TOO_SHORT
      return get_login(NULL, );
    case VALRES_TOO_LONG:
      // SIGNUPERR_UNAME_TOO_LONG
      return get_login(NULL, );
    case VALRES_INV_CHAR:
      // SIGNUPERR_UNAME_INV_CHAR
      return get_login(NULL, );
    default:
      log_crit("post_signup(): unexpected validation result value for username: %d", uname_valid);
  }

  enum validation_result passwd_valid = valid_user_name(passwd);
  switch (passwd_valid)
  {
    case VALRES_OK:
      break;
    case VALRES_TOO_SHORT:
      // SIGNUPERR_PASSWD_TOO_SHORT
      return get_login(NULL, );
    case VALRES_TOO_LONG:
      // SIGNUPERR_PASSWD_TOO_LONG
      return get_login(NULL, );
    case VALRES_UNMET:
      // SIGNUPERR_PASSWD_UNMET
      return get_login(NULL, );
    case VALRES_INV_ENC:
      // SIGNUPERR_INV_ENC
      return get_login(NULL, );
    default:
      log_crit("post_signup(): unexpected validation result value for username: %d", passwd_valid);
  }

  // create new user
  const char* token;
  if ((token = new_user(uname, passwd, about)) == NULL)
  {
    return get_login(NULL, SIGNUPERR_UNAME_TAKEN);
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
    return response_error(STAT404);
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


static struct response* post_comment(const char* post_id, const char* community_id, const char* body, const struct auth_token* client_info)
{
  if (client_info == NULL)
  {
    return response_redirect("/login");
  }

  if (post_id == NULL || community_id == NULL)
  {
    
  }

  return NULL;
}
