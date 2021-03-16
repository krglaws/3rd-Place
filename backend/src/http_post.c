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
static struct response* post_comment(const char* post_id, const char* body, const struct auth_token* client_info);
static struct response* post_post(const char* community_name, const char* post_title, const char* post_body, const struct auth_token* client_info);
static struct response* post_community(const char* community_name, const char* community_about, const struct auth_token* client_info);


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
    const char* body = get_map_value_str(req->content, "body");

    return post_comment(post_id, body, req->client_info);
  }

  else if (strcmp(req->uri, "./new_post") == 0)
  {
    const char* community_name = get_map_value_str(req->content, "community_name");
    const char* post_title = get_map_value_str(req->content, "post_title");
    const char* post_body = get_map_value_str(req->content, "post_body");

    return post_post(community_name, post_title, post_body, req->client_info);
  }

  else if (strcmp(req->uri, "./new_community") == 0)
  {
    const char* community_name = get_map_value_str(req->content, "community_name");
    const char* community_about = get_map_value_str(req->content, "community_about");

    return post_community(community_name, community_about, req->client_info);
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

  // Not using USER_PASSWD_BUF_LEN since it is only used for the password hash
  char passwd_decoded[MAX_PASSWD_LEN + 1];

  if (uname == NULL || passwd == NULL ||
      validate_passwd(passwd_decoded, passwd) != VALRES_OK ||
      (token = login_user(uname, passwd_decoded)) == NULL)
  {
    // no need to indicate exactly why login failed
    return get_login(USER_FORM_ERR_BAD_LOGIN, NULL);
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

  enum validation_result valres = validate_user_name(uname);

  switch (valres)
  {
    case VALRES_OK:
      break;
    case VALRES_TOO_SHORT:
      return get_login(USER_FORM_ERR_UNAME_TOO_SHORT, NULL);
    case VALRES_TOO_LONG:
      // SIGNUPERR_UNAME_TOO_LONG
      return get_login(USER_FORM_ERR_UNAME_TOO_LONG, NULL);
    case VALRES_INV_CHAR:
      // SIGNUPERR_UNAME_INV_CHAR
      return get_login(USER_FORM_ERR_UNAME_INV_CHAR, NULL);
    default:
      log_crit("post_signup(): unexpected validation result value for username: '%s', errno: %d", uname, valres);
  }

  char passwd_decoded[USER_PASSWD_BUF_LEN];
  valres = validate_passwd(passwd_decoded, passwd);

  switch (valres)
  {
    case VALRES_OK:
      break;
    case VALRES_TOO_SHORT:
      return get_login(USER_FORM_ERR_PASSWD_TOO_SHORT, NULL);
    case VALRES_TOO_LONG:
      return get_login(USER_FORM_ERR_PASSWD_TOO_LONG, NULL);
    case VALRES_UNMET:
      return get_login(USER_FORM_ERR_PASSWD_UNMET, NULL);
    case VALRES_INV_ENC:
      return get_login(USER_FORM_ERR_PASSWD_INV_ENC, NULL);
    default:
      log_crit("post_signup(): unexpected validation result value for passwd: '%s', errno: %d", passwd, valres);
  }

  char about_decoded[USER_ABOUT_BUF_LEN];
  valres = validate_user_about(about_decoded, about);

  switch (valres)
  {
    case VALRES_OK:
      break;
    case VALRES_TOO_SHORT:
      return get_login(USER_FORM_ERR_ABOUT_TOO_SHORT, NULL);
    case VALRES_TOO_LONG:
      return get_login(USER_FORM_ERR_ABOUT_TOO_LONG, NULL);
    case VALRES_INV_ENC:
      return get_login(USER_FORM_ERR_ABOUT_INV_ENC, NULL);
    default:
      log_crit("post_signup(): unexpected validation result value for about: '%s', errno: %d", about, valres);
  }

  // create new user
  const char* token;
  if ((token = new_user(uname, passwd, about)) == NULL)
  {
    return get_login(USER_FORM_ERR_UNAME_TAKEN, NULL);
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
    else return response_error(STAT404);
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
    else return response_error(STAT404);
  }

  // prepare 200 response object
  struct response* resp = calloc(1, sizeof(struct response));
  resp->header = ks_list_new();
  ks_list_add(resp->header, ks_datacont_new(STAT200, KS_CHARP, strlen(STAT200)));

  char* contlen = "Content-Length: 0\r\n";
  ks_list_add(resp->header, ks_datacont_new(contlen, KS_CHARP, strlen(contlen)));

  return resp;
}


static struct response* post_comment(const char* post_id, const char* body, const struct auth_token* client_info)
{
  if (client_info == NULL)
  {
    return response_redirect("/login");
  }

  // make sure post exists
  ks_hashmap* post_info;
  if ((post_info = query_post_by_id(post_id)) == NULL)
  {
    return response_error(STAT404);
  }
  ks_hashmap_delete(post_info);

  char body_decoded[COMMENT_BODY_BUF_LEN + 1];
  enum validation_result valres = validate_comment_body(body_decoded, body);

  switch (valres)
  {
    case VALRES_OK:
      break;
    case VALRES_INV_ENC:
      return get_new_comment(post_id, COMMENT_FORM_ERR_INV_ENC, client_info);
    case VALRES_TOO_LONG:
      return get_new_comment(post_id, COMMENT_FORM_ERR_TOO_LONG, client_info);
    case VALRES_TOO_SHORT:
      return get_new_comment(post_id, COMMENT_FORM_ERR_TOO_SHORT, client_info);
    default:
      log_crit("post_comment(): unexpected validation result value for comment: '%s', errno: %d", body, valres);
  }

  // create comment in database
  char* comment_id;
  if ((comment_id = sql_create_comment(client_info->user_id, post_id, body)) == NULL)
  {
    log_err("post_comment(): failed on call to sql_create_comment()");
    return response_error(STAT500);
  }

  // build comment URI
  char uri[32];
  sprintf(uri, "/p/%s#%s", post_id, comment_id);
  free(comment_id);

  return response_redirect(uri);
}


static struct response* post_post(const char* community_name, const char* post_title, const char* post_body, const struct auth_token* client_info)
{
  if (client_info == NULL)
  {
    return response_redirect("/login");
  }

  // make sure community exists
  ks_hashmap* community_info;
  if ((community_info = query_community_by_name(community_name)) == NULL)
  {
    return response_error(STAT404);
  }
  ks_hashmap_delete(community_info);

  char title_decoded[POST_TITLE_BUF_LEN];
  enum validation_result valres = validate_post_title(title_decoded, post_title);

  switch (valres)
  {
    case VALRES_OK:
      break;
    case VALRES_INV_ENC:
      return get_new_post(community_name, POST_FORM_ERR_TITLE_INV_ENC, client_info);
    case VALRES_TOO_LONG:
      return get_new_post(community_name, POST_FORM_ERR_TITLE_TOO_LONG, client_info);
    case VALRES_TOO_SHORT:
      return get_new_post(community_name, POST_FORM_ERR_TITLE_TOO_SHORT, client_info);
    default:
      log_crit("post_post(): unexpected validation result value for post title: '%s', errno: %d", post_title, valres);
  }

  char body_decoded[POST_BODY_BUF_LEN];
  valres = validate_post_body(body_decoded, post_body);

  switch (valres)
  {
    case VALRES_OK:
      break;
    case VALRES_INV_ENC:
      return get_new_post(community_name, POST_FORM_ERR_BODY_INV_ENC, client_info);
    case VALRES_TOO_LONG:
      return get_new_post(community_name, POST_FORM_ERR_BODY_TOO_LONG, client_info);
    case VALRES_TOO_SHORT:
      return get_new_post(community_name, POST_FORM_ERR_BODY_TOO_SHORT, client_info);
    default:
      log_crit("post_post(): unexpected validation result value for post body: '%s', errno: %d", post_body, valres);
  }


  // create comment in database
  const char* community_id = get_map_value_str(community_info, FIELD_COMMUNITY_ID);
  char* post_id;
  if ((post_id = sql_create_post(client_info->user_id, community_id, post_title, post_body)) == NULL)
  {
    log_err("post_post(): failed on call to sql_create_post()");
    return response_error(STAT500);
  }

  // build comment URI
  char uri[32];
  sprintf(uri, "/p/%s", post_id);
  free(post_id);

  return response_redirect(uri);
}


static struct response* post_community(const char* community_name, const char* community_about, const struct auth_token* client_info)
{
  if (client_info == NULL)
  {
    return response_redirect("/login");
  }

  enum validation_result valres = validate_community_name(community_name);

  switch (valres)
  {
    case VALRES_OK:
      break;
    case VALRES_TOO_LONG:
      return get_new_community(COMMUNITY_FORM_ERR_NAME_TOO_LONG, client_info);
    case VALRES_TOO_SHORT:
      return get_new_community(COMMUNITY_FORM_ERR_NAME_TOO_SHORT, client_info);
    case VALRES_INV_CHAR:
      return get_new_community(COMMUNITY_FORM_ERR_NAME_INV_CHAR, client_info);
    default:
      log_crit("post_community(): unexpected validation result value for community name: '%s', errno: %d", community_name, valres);
  }

  char about_decoded[COMMUNITY_ABOUT_BUF_LEN];
  valres = validate_community_about(about_decoded, community_about);

  switch (valres)
  {
    case VALRES_OK:
      break;
    case VALRES_TOO_LONG:
      return get_new_community(COMMUNITY_FORM_ERR_ABOUT_TOO_LONG, client_info);
    case VALRES_TOO_SHORT:
      return get_new_community(COMMUNITY_FORM_ERR_ABOUT_TOO_SHORT, client_info);
    case VALRES_INV_ENC:
      return get_new_community(COMMUNITY_FORM_ERR_ABOUT_INV_ENC, client_info);
    default:
      log_crit("post_community(): unexpected validation result value for commuity about: '%s', errno: %d", community_about, valres);
  }

  char* post_id;
  if ((post_id = sql_create_community(client_info->user_id, community_name, about_decoded)) == NULL)
  {
    log_err("post_community(): failed on call to sql_create_community()");
    return response_error(STAT500);
  }

  // build post URI
  char uri[32];
  sprintf(uri, "/p/%s", post_id);
  free(post_id);

  return response_redirect(uri);
}

