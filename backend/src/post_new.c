//#define _XOPEN_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <kylestructs.h>

#include <log_manager.h>
#include <auth_manager.h>
#include <sql_manager.h>
#include <validation.h>
#include <string_map.h>
#include <response.h>
#include <server.h>
#include <get_form.h>
#include <http_get.h>
#include <post_new.h>


/* used for successful login/signup responses */
#define COOKIE_TEMPLATE "Set-Cookie: auth=%s; Expires=Wed, 1 Jan 2121 00:00:00 GMT; SameSite=Strict\r\n"


struct response* login(const struct request* req)
{
  if (req->client_info != NULL)
  {
    // this will pull up the "you are already logged in" message
    return response_redirect("/login");
  }

  // get args
  const char* user_name = get_map_value_str(req->content, "username");
  const char* password = get_map_value_str(req->content, "password");
  const char* token;

  // Not using USER_PASSWD_BUF_LEN since it is only used for the password hash
  char password_decoded[MAX_PASSWD_LEN + 1];

  if (user_name == NULL || password == NULL ||
      validate_password(password_decoded, password) != VALRES_OK ||
      (token = login_user(user_name, password_decoded)) == NULL)
  {
    // no need to indicate exactly why login failed
    return get_login_internal(NULL, USER_FORM_ERR_BAD_LOGIN, NULL);
  }

  // build redirect
  struct response* resp = response_redirect("/home");

  // add token to response
  char token_hdr[128];
  sprintf(token_hdr, COOKIE_TEMPLATE, token);
  ks_list_add(resp->header, ks_datacont_new(token_hdr, KS_CHARP, strlen(token_hdr)));

  return resp;
}


struct response* logout(const struct request* req)
{
  if (req->client_info != NULL)
  {
    remove_token(req->client_info->token);
  }

  return response_redirect("/home");
}


struct response* signup(const struct request* req)
{
  if (req->client_info != NULL)
  {
    // this will pull up the "you are already logged in" message
    return response_redirect("/login");
  }

  // get args
  const char* user_name = get_map_value_str(req->content, "username");
  const char* password1 = get_map_value_str(req->content, "password1");
  const char* password2 = get_map_value_str(req->content, "password2");

  enum validation_result valres = validate_user_name(user_name);

  switch (valres)
  {
    case VALRES_OK:
      break;
    case VALRES_TOO_SHORT:
      return get_login_internal(NULL, USER_FORM_ERR_UNAME_TOO_SHORT, NULL);
    case VALRES_TOO_LONG:
      return get_login_internal(NULL, USER_FORM_ERR_UNAME_TOO_LONG, NULL);
    case VALRES_INV_CHAR:
      return get_login_internal(NULL, USER_FORM_ERR_UNAME_INV_CHAR, NULL);
    default:
      log_crit("post_signup(): unexpected validation result value for username: '%s', errno: %d", user_name, valres);
  }

  if (password1 == NULL && password2 == NULL)
  {
    return get_login_internal(NULL, USER_FORM_ERR_PASSWD_TOO_SHORT, NULL);
  }

  if (password1 == NULL || password2 == NULL ||
      strcmp(password1, password2) != 0)
  {
    return get_login_internal(NULL, USER_FORM_ERR_PASSWD_MISMATCH, NULL);
  }

  char password_decoded[USER_PASSWD_BUF_LEN];
  valres = validate_password(password_decoded, password1);

  switch (valres)
  {
    case VALRES_OK:
      break;
    case VALRES_TOO_SHORT:
      return get_login_internal(NULL, USER_FORM_ERR_PASSWD_TOO_SHORT, NULL);
    case VALRES_TOO_LONG:
      return get_login_internal(NULL, USER_FORM_ERR_PASSWD_TOO_LONG, NULL);
    case VALRES_UNMET:
      return get_login_internal(NULL, USER_FORM_ERR_PASSWD_UNMET, NULL);
    case VALRES_INV_ENC:
      return get_login_internal(NULL, USER_FORM_ERR_PASSWD_INV_ENC, NULL);
    default:
      log_crit("post_signup(): unexpected validation result value for password: '%s', errno: %d", password1, valres);
  }

  // create new user
  const char* token;
  if ((token = new_user(user_name, password1)) == NULL)
  {
    return get_login_internal(user_name, USER_FORM_ERR_UNAME_TAKEN, NULL);
  }

  // build redirect
  struct response* resp = response_redirect("/home");
  char token_hdr[128];
  sprintf(token_hdr, COOKIE_TEMPLATE, token);
  ks_list_add(resp->header, ks_datacont_new(token_hdr, KS_CHARP, strlen(token_hdr)));

  return resp;
}


struct response* vote(const struct request* req)
{
  if (req->client_info == NULL)
  {
    return response_redirect("/login");
  }

  // get args
  const char* type = get_map_value_str(req->content, "type");
  const char* direction = get_map_value_str(req->content, "direction");
  const char* id = get_map_value_str(req->content, "id");

  if (type == NULL || direction == NULL || id == NULL)
  {
    return response_error(STAT404);
  }

  if (strcmp(type, "post") == 0)
  {
    if (strcmp(direction, "up") == 0)
    {
      if (sql_toggle_post_up_vote(id, req->client_info->user_id) != 0)
      {
        return response_error(STAT500);
      }
    }
    else if (strcmp(direction, "down") == 0)
    {
      if (sql_toggle_post_down_vote(id, req->client_info->user_id) != 0)
      {
        return response_error(STAT500);
      }
    }
    else return response_error(STAT404);
  }
  else if (strcmp(type, "comment") == 0)
  {
    if (strcmp(direction, "up") == 0)
    {
      if (sql_toggle_comment_up_vote(id, req->client_info->user_id) != 0)
      {
        return response_error(STAT500);
      }
    }
    else if (strcmp(direction, "down") == 0)
    {
      if (sql_toggle_comment_down_vote(id, req->client_info->user_id) != 0)
      {
        return response_error(STAT500);
      }
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


struct response* subscribe(const struct request* req)
{
  if (req->client_info == NULL)
  {
    return response_redirect("/login");
  }

  // get args
  const char* community_id = get_map_value_str(req->content, "id");

  ks_hashmap* community_info;
  if (community_id == NULL || (community_info = query_community_by_id(community_id)) == NULL)
  {
    return response_error(STAT404);
  }
  ks_hashmap_delete(community_info);

  if (sql_toggle_subscription(community_id, req->client_info->user_id) != 0)
  {
    return response_error(STAT500);
  }

  // prepare 200 response object
  struct response* resp = calloc(1, sizeof(struct response));
  resp->header = ks_list_new();
  ks_list_add(resp->header, ks_datacont_new(STAT200, KS_CHARP, strlen(STAT200)));

  char* contlen = "Content-Length: 0\r\n";
  ks_list_add(resp->header, ks_datacont_new(contlen, KS_CHARP, strlen(contlen)));

  return resp;
}


struct response* new_comment(const struct request* req)
{
  if (req->client_info == NULL)
  {
    return response_redirect("/login");
  }

  // get args
  const char* post_id = get_map_value_str(req->content, "id");
  const char* comment_body = get_map_value_str(req->content, "body");

  // make sure post exists
  ks_hashmap* post_info;
  if ((post_info = query_post_by_id(post_id)) == NULL)
  {
    return response_error(STAT404);
  }
  ks_hashmap_delete(post_info);

  char body_decoded[COMMENT_BODY_BUF_LEN + 1];
  enum validation_result valres = validate_comment_body(body_decoded, comment_body);

  switch (valres)
  {
    case VALRES_OK:
      break;
    case VALRES_INV_ENC:
      return get_new_comment_internal(NULL, post_id, COMMENT_FORM_ERR_INV_ENC, req->client_info);
    case VALRES_TOO_LONG:
      return get_new_comment_internal(NULL, post_id, COMMENT_FORM_ERR_TOO_LONG, req->client_info);
    case VALRES_TOO_SHORT:
      return get_new_comment_internal(NULL, post_id, COMMENT_FORM_ERR_TOO_SHORT, req->client_info);
    default:
      log_crit("post_comment(): unexpected validation result value for comment: '%s', errno: %d", comment_body, valres);
  }

  // create comment in database
  char* comment_id;
  if ((comment_id = sql_create_comment(req->client_info->user_id, post_id, body_decoded)) == NULL)
  {
    log_err("post_comment(): failed on call to sql_create_comment()");
    return response_error(STAT500);
  }

  // build comment URI
  char uri[64];
  sprintf(uri, "/post?id=%s#%s", post_id, comment_id);
  free(comment_id);

  return response_redirect(uri);
}


struct response* new_post(const struct request* req)
{
  if (req->client_info == NULL)
  {
    return response_redirect("/login");
  }

  // get args
  const char* community_id = get_map_value_str(req->content, "id");
  const char* post_title = get_map_value_str(req->content, "title");
  const char* post_body = get_map_value_str(req->content, "body");

  // make sure community exists
  ks_hashmap* community_info;
  if ((community_info = query_community_by_id(community_id)) == NULL)
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
      return get_new_post_internal(NULL, NULL, community_id, POST_FORM_ERR_TITLE_INV_ENC, req->client_info);
    case VALRES_TOO_LONG:
      return get_new_post_internal(NULL, NULL, community_id, POST_FORM_ERR_TITLE_TOO_LONG, req->client_info);
    case VALRES_TOO_SHORT:
      return get_new_post_internal(NULL, NULL, community_id, POST_FORM_ERR_TITLE_TOO_SHORT, req->client_info);
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
      return get_new_post_internal(NULL, NULL, community_id, POST_FORM_ERR_BODY_INV_ENC, req->client_info);
    case VALRES_TOO_LONG:
      return get_new_post_internal(NULL, NULL, community_id, POST_FORM_ERR_BODY_TOO_LONG, req->client_info);
    case VALRES_TOO_SHORT:
      return get_new_post_internal(NULL, NULL, community_id, POST_FORM_ERR_BODY_TOO_SHORT, req->client_info);
    default:
      log_crit("post_post(): unexpected validation result value for post body: '%s', errno: %d", post_body, valres);
  }

  // create comment in database
  char* post_id;
  if ((post_id = sql_create_post(req->client_info->user_id, community_id, title_decoded, body_decoded)) == NULL)
  {
    log_err("post_post(): failed on call to sql_create_post()");
    return response_error(STAT500);
  }

  // build comment URI
  char uri[64];
  sprintf(uri, "/post?id=%s", post_id);
  free(post_id);

  return response_redirect(uri);
}


struct response* new_community(const struct request* req)
{
  if (req->client_info == NULL)
  {
    return response_redirect("/login");
  }

  const char* community_name = get_map_value_str(req->content, "name");
  const char* community_about = get_map_value_str(req->content, "about");

  enum validation_result valres = validate_community_name(community_name);

  switch (valres)
  {
    case VALRES_OK:
      break;
    case VALRES_TOO_LONG:
      return get_new_community_internal(NULL, NULL, COMMUNITY_FORM_ERR_NAME_TOO_LONG, req->client_info);
    case VALRES_TOO_SHORT:
      return get_new_community_internal(NULL, NULL, COMMUNITY_FORM_ERR_NAME_TOO_SHORT, req->client_info);
    case VALRES_INV_CHAR:
      return get_new_community_internal(NULL, NULL, COMMUNITY_FORM_ERR_NAME_INV_CHAR, req->client_info);
    default:
      log_crit("post_community(): unexpected validation result value for community name: '%s', errno: %d", community_name, valres);
  }

  ks_hashmap* community_info;
  if ((community_info = query_community_by_name(community_name)) != NULL)
  {
    ks_hashmap_delete(community_info);
    return get_new_community_internal(NULL, NULL, COMMUNITY_FORM_ERR_NAME_ALREADY_EXISTS, req->client_info);
  }

  char about_decoded[COMMUNITY_ABOUT_BUF_LEN];
  valres = validate_community_about(about_decoded, community_about);

  switch (valres)
  {
    case VALRES_OK:
      break;
    case VALRES_TOO_LONG:
      return get_new_community_internal(NULL, NULL, COMMUNITY_FORM_ERR_ABOUT_TOO_LONG, req->client_info);
    case VALRES_TOO_SHORT:
      return get_new_community_internal(NULL, NULL, COMMUNITY_FORM_ERR_ABOUT_TOO_SHORT, req->client_info);
    case VALRES_INV_ENC:
      return get_new_community_internal(NULL, NULL, COMMUNITY_FORM_ERR_ABOUT_INV_ENC, req->client_info);
    default:
      log_crit("post_community(): unexpected validation result value for commuity about: '%s', errno: %d", community_about, valres);
  }

  // create community
  char* community_id;
  if ((community_id = sql_create_community(req->client_info->user_id, community_name, about_decoded)) == NULL)
  {
    log_err("post_community(): failed on call to sql_create_community()");
    return response_error(STAT500);
  }

  // subscribe to community
  if (sql_toggle_subscription(community_id, req->client_info->user_id) == -1)
  {
    log_err("post_community(): failed on call to sql_toggle_subscription()");
  }

  // build post URI
  char uri[64];
  sprintf(uri, "/community?id=%s", community_id);
  free(community_id);

  return response_redirect(uri);
}


struct response* new_moderator(const struct request* req)
{
  if (req->client_info == NULL)
  {
    return response_redirect("/login");
  }

  const char* community_id = get_map_value_str(req->content, "id");
  const char* user_name = get_map_value_str(req->content, "name");

  // check if community exists
  ks_hashmap* community_info;
  if ((community_info = query_community_by_id(community_id)) == NULL)
  {
    return response_error(STAT404);
  }

  // make sure client has permission
  const char* owner_id = get_map_value_str(community_info, FIELD_COMMUNITY_OWNER_ID);
  if (strcmp(owner_id, req->client_info->user_id) != 0)
  {
    ks_hashmap_delete(community_info);
    return get_edit_community_internal(NULL, community_id, COMMUNITY_FORM_ERR_MOD_OWNER_ONLY, req->client_info);
  }
  ks_hashmap_delete(community_info);

  // check if user exists
  ks_hashmap* user_info;
  if ((user_info = query_user_by_name(user_name)) == NULL)
  {
    return get_edit_community_internal(NULL, community_id, COMMUNITY_FORM_ERR_MOD_USER_NOT_EXIST, req->client_info);
  }

  // check if user is already a mod
  const char* user_id = get_map_value_str(user_info, FIELD_USER_ID);
  ks_hashmap* mod_info;
  if ((mod_info = query_moderator_by_user_id_community_id(user_id, community_id)) != NULL)
  {
    ks_hashmap_delete(user_info);
    ks_hashmap_delete(mod_info);
    return get_edit_community_internal(NULL, community_id, COMMUNITY_FORM_ERR_MOD_USER_ALREADY, req->client_info);
  }

  // add mod entry to DB
  if (sql_create_moderator(user_id, community_id) == -1)
  {
    ks_hashmap_delete(user_info);
    return response_error(STAT500);
  }
  ks_hashmap_delete(user_info);

  char uri[64];
  sprintf(uri, "/community?id=%s", community_id);
  return response_redirect(uri);
}
