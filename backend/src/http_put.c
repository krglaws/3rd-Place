#include <stdio.h>
#include <string.h>
#include <kylestructs.h>

#include <string_map.h>
#include <log_manager.h>
#include <sql_manager.h>
#include <auth_manager.h>
#include <response.h>
#include <validator.h>
#include <get_form.h>
#include <http_put.h>


static struct response* update_user_about(const char* about, const struct auth_token* client_info);
static struct response* update_user_password(const char* oldpass, const char* newpass1, const char* newpass2, const struct auth_token* client_info);
static struct response* update_post(const char* id, const char* body, const struct auth_token* client_info);
static struct response* update_comment(const char* id, const char* body, const struct auth_token* client_info);
static struct response* update_community_about(const char* id, const char* about, const struct auth_token* client_info);


struct response* http_put(struct request* req)
{
  if (strcmp(req->uri, "./user_about") == 0)
  {
    const char* about = get_map_value_str(req->content, "about");

    return update_user_about(about, req->client_info);
  }

  if (strcmp(req->uri, "./user_password") == 0)
  {
    const char* oldpass = get_map_value_str(req->content, "oldpass");
    const char* newpass1 = get_map_value_str(req->content, "newpass1");
    const char* newpass2 = get_map_value_str(req->content, "newpass2");

    return update_user_password(oldpass, newpass1, newpass2, req->client_info);
  }

  if (strcmp(req->uri, "./post") == 0)
  {
    const char* id = get_map_value_str(req->content, "id");
    const char* body = get_map_value_str(req->content, "body");

    return update_post(id, body, req->client_info);
  }

  if (strcmp(req->uri, "./comment") == 0)
  {
    const char* id = get_map_value_str(req->content, "id");
    const char* body = get_map_value_str(req->content, "body");

    return update_comment(id, body, req->client_info);
  }

  if (strcmp(req->uri, "./community_about") == 0)
  {
    const char* id = get_map_value_str(req->content, "id");
    const char* about = get_map_value_str(req->content, "about");

    return update_community_about(id, about, req->client_info);
  }

  return response_error(STAT404);
}


static struct response* update_user_about(const char* about, const struct auth_token* client_info)
{
  if (client_info == NULL)
  {
    return response_redirect("/login");
  }

  char about_decoded[USER_ABOUT_BUF_LEN];
  enum validation_result valres = validate_user_about(about_decoded, about);

  switch (valres)
  {
    case VALRES_OK:
      break;
    case VALRES_TOO_SHORT:
      return get_edit_user(about_decoded, USER_FORM_ERR_ABOUT_TOO_SHORT, client_info);
    case VALRES_TOO_LONG:
      return get_edit_user(about_decoded, USER_FORM_ERR_ABOUT_TOO_LONG, client_info);
    case VALRES_INV_ENC:
      return get_edit_user(about_decoded, USER_FORM_ERR_ABOUT_INV_ENC, client_info);
    default:
      log_crit("update_user_about(): invalid validation error for user about: %d", valres);
  }

  // submit data to database
  if (sql_update_user_about(client_info->user_id, about_decoded) == -1)
  {
    // server error
    return response_error(STAT500);
  }

  char uri[32];
  sprintf(uri, "/u/%s", client_info->user_name);

  // redirect to user page
  return response_redirect(uri);
}


static struct response* update_user_password(const char* oldpass, const char* newpass1, const char* newpass2, const struct auth_token* client_info)
{
  if (client_info == NULL)
  {
    return response_redirect("/login");
  }

  // make sure oldpass is correct
  char oldpass_decoded[USER_PASSWD_BUF_LEN];
  if (validate_password(oldpass_decoded, oldpass) != VALRES_OK ||
      login_user(client_info->user_name, oldpass_decoded) == NULL)
  {
    return get_edit_user(NULL, USER_FORM_ERR_BAD_LOGIN, client_info);
  }

  // make sure new password inputs match
  if (newpass1 == NULL && newpass2 == NULL)
  {
    return get_edit_user(NULL, USER_FORM_ERR_PASSWD_TOO_SHORT, client_info);
  }

  if (newpass1 == NULL || newpass2 == NULL ||
      strcmp(newpass1, newpass2) != 0)
  {
    return get_edit_user(NULL, USER_FORM_ERR_PASSWD_MISMATCH, client_info);
  }

  // validate new password
  char newpass_decoded[USER_PASSWD_BUF_LEN];
  enum validation_result valres = validate_password(newpass_decoded, newpass1); 

  switch (valres)
  {
    case VALRES_OK:
      break;
    case VALRES_TOO_SHORT:
      return get_edit_user(NULL, USER_FORM_ERR_PASSWD_TOO_SHORT, client_info);
    case VALRES_TOO_LONG:
      return get_edit_user(NULL, USER_FORM_ERR_PASSWD_TOO_LONG, client_info);
    case VALRES_UNMET:
      return get_edit_user(NULL, USER_FORM_ERR_PASSWD_UNMET, client_info);
    case VALRES_INV_ENC:
      return get_edit_user(NULL, USER_FORM_ERR_PASSWD_INV_ENC, client_info);
    default:
      log_crit("update_user_password(): invalid validation error for user password: %d", valres);
  }

  // update user password
  if (update_password(client_info->user_id, newpass_decoded) == -1)
  {
    return response_error(STAT500);
  }

  char uri[32];
  sprintf(uri, "/u/%s", client_info->user_name);

  // redirect to user page
  return response_redirect(uri);
}


static struct response* update_post(const char* id, const char* body, const struct auth_token* client_info)
{
  if (client_info == NULL)
  {
    return response_redirect("/login");
  }

  // check if post exists
  ks_hashmap* post_info;
  if ((post_info = query_post_by_id(id)) == NULL)
  {
    return response_error(STAT404);
  }

  // check if client owns this post
  const char* author_id = get_map_value_str(post_info, FIELD_POST_AUTHOR_ID);
  if (strcmp(author_id, client_info->user_id) != 0)
  {
    // permission denied
    return response_error(STAT400);
  }
  ks_hashmap_delete(post_info);

  // validate post body
  char body_decoded[POST_BODY_BUF_LEN];
  enum validation_result valres = validate_post_body(body_decoded, body);

  switch (valres)
  {
    case VALRES_OK:
      break;
    case VALRES_TOO_SHORT:
      return get_edit_post(body_decoded, id, POST_FORM_ERR_BODY_TOO_SHORT, client_info);
    case VALRES_TOO_LONG:
      return get_edit_post(body_decoded, id, POST_FORM_ERR_BODY_TOO_LONG, client_info);
    case VALRES_INV_ENC:
      return get_edit_post(body_decoded, id, POST_FORM_ERR_BODY_INV_ENC, client_info);
    default:
      log_crit("update_post(): invalid validation error for post body: %d", valres);
  }

  // update post body
  if (sql_update_post_body(id, body_decoded) == -1)
  {
    return response_error(STAT500);
  }

  char uri[32];
  sprintf(uri, "/p/%s", id);

  // redirect to post page
  return response_redirect(uri);
}


static struct response* update_comment(const char* id, const char* body, const struct auth_token* client_info)
{
  if (client_info == NULL)
  {
    return response_redirect("/login");
  }

  // check if comment exists
  ks_hashmap* comment_info;
  if ((comment_info = query_comment_by_id(id)) == NULL)
  {
    return response_error(STAT404);
  }

  // check if client owns this comment
  const char* author_id = get_map_value_str(comment_info, FIELD_COMMENT_AUTHOR_ID);
  if (strcmp(author_id, client_info->user_id) != 0)
  {
    return response_error(STAT400);
  }

  // validate comment body
  char body_decoded[COMMENT_BODY_BUF_LEN];
  enum validation_result valres = validate_comment_body(body_decoded, body);

  switch (valres)
  {
    case VALRES_OK:
      break;
    case VALRES_TOO_SHORT:
      return get_edit_comment(body_decoded, id, COMMENT_FORM_ERR_TOO_SHORT, client_info);
    case VALRES_TOO_LONG:
      return get_edit_comment(body_decoded, id, COMMENT_FORM_ERR_TOO_LONG, client_info);
    case VALRES_INV_ENC:
      return get_edit_comment(body_decoded, id, COMMENT_FORM_ERR_INV_ENC, client_info);
    default:
      log_crit("update_comment(): invalid validation error for comment body: %d", valres);
  }

  // update comment body
  if (sql_update_comment_body(id, body_decoded) == -1)
  {
    ks_hashmap_delete(comment_info);
    return response_error(STAT500);
  }

  const char* post_id = get_map_value_str(comment_info, FIELD_COMMENT_POST_ID);
  char uri[32];
  sprintf(uri, "/p/%s#%s", post_id, id);
  ks_hashmap_delete(comment_info);

  return response_redirect(uri);
}


static struct response* update_community_about(const char* id, const char* about, const struct auth_token* client_info)
{
  if (client_info == NULL)
  {
    return response_redirect("/login");
  }

  // check if community exists
  ks_hashmap* community_info;
  if ((community_info = query_community_by_id(id)) == NULL)
  {
    return response_error(STAT404);
  }

  // check if client owns this community
  const char* owner_id = get_map_value_str(community_info, FIELD_COMMUNITY_OWNER_ID);
  if (strcmp(owner_id, client_info->user_id) != 0)
  {
    // check if client is a mod
    ks_hashmap* mod_info;
    if ((mod_info = query_moderator_by_community_id_user_id(id, client_info->user_id)) == NULL)
    {
      // check if client is an admin
      ks_hashmap* admin_info;
      if ((admin_info = query_administrator_by_user_id(client_info->user_id)) == NULL)
      {
        ks_hashmap_delete(community_info);
        return response_error(STAT400);
      }
      ks_hashmap_delete(admin_info);
    }
    ks_hashmap_delete(mod_info);
  }
  ks_hashmap_delete(community_info);

  // validate about
  char about_decoded[COMMUNITY_ABOUT_BUF_LEN];
  enum validation_result valres = validate_community_about(about_decoded, about);

  switch (valres)
  {
    case VALRES_OK:
      break;
    case VALRES_TOO_SHORT:
      return get_edit_community(about_decoded, id, COMMUNITY_FORM_ERR_ABOUT_TOO_SHORT, client_info);
    case VALRES_TOO_LONG:
      return get_edit_community(about_decoded, id, COMMUNITY_FORM_ERR_ABOUT_TOO_LONG, client_info);
    case VALRES_INV_ENC:
      return get_edit_community(about_decoded, id, COMMUNITY_FORM_ERR_ABOUT_INV_ENC, client_info);
    default:
      log_crit("update_community_about(): invalid validation error for community about: %d", valres);
  }

  // update community about
  if (sql_update_community_about(id, about_decoded) == -1)
  {
    return response_error(STAT500);
  }

  char uri[32];
  sprintf(uri, "/c/%s", id);

  return response_redirect(uri);
}
