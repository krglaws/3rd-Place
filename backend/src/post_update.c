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
#include <post_update.h>


struct response* update_user_about(const struct request* req)
{
  if (req->client_info == NULL)
  {
    return response_redirect("/login");
  }

  const char* about = get_map_value_str(req->content, "about"); 

  char about_decoded[USER_ABOUT_BUF_LEN];
  enum validation_result valres = validate_user_about(about_decoded, about);

  switch (valres)
  {
    case VALRES_OK:
      break;
    case VALRES_TOO_SHORT:
      return get_edit_user_internal(about_decoded, USER_FORM_ERR_ABOUT_TOO_SHORT, req->client_info);
    case VALRES_TOO_LONG:
      return get_edit_user_internal(about_decoded, USER_FORM_ERR_ABOUT_TOO_LONG, req->client_info);
    case VALRES_INV_ENC:
      return get_edit_user_internal(about_decoded, USER_FORM_ERR_ABOUT_INV_ENC, req->client_info);
    default:
      log_crit("update_user_about(): invalid validation error for user about: %d", valres);
  }

  // submit data to database
  if (sql_update_user_about(req->client_info->user_id, about_decoded) == -1)
  {
    // server error
    return response_error(STAT500);
  }

  char uri[64];
  sprintf(uri, "/user?id=%s", req->client_info->user_id);

  // redirect to user page
  return response_redirect(uri);
}


struct response* update_user_password(const struct request* req)
{
  if (req->client_info == NULL)
  {
    return response_redirect("/login");
  }

  const char* oldpass = get_map_value_str(req->content, "oldpass");
  const char* newpass1 = get_map_value_str(req->content, "newpass1");
  const char* newpass2 = get_map_value_str(req->content, "newpass2");

  // make sure oldpass is correct
  char oldpass_decoded[USER_PASSWD_BUF_LEN];
  if (validate_password(oldpass_decoded, oldpass) != VALRES_OK ||
      login_user(req->client_info->user_name, oldpass_decoded) == NULL)
  {
    return get_edit_user_internal(NULL, USER_FORM_ERR_BAD_LOGIN, req->client_info);
  }

  // make sure new password inputs match
  if (newpass1 == NULL && newpass2 == NULL)
  {
    return get_edit_user_internal(NULL, USER_FORM_ERR_PASSWD_TOO_SHORT, req->client_info);
  }

  if (newpass1 == NULL || newpass2 == NULL ||
      strcmp(newpass1, newpass2) != 0)
  {
    return get_edit_user_internal(NULL, USER_FORM_ERR_PASSWD_MISMATCH, req->client_info);
  }

  // validate new password
  char newpass_decoded[USER_PASSWD_BUF_LEN];
  enum validation_result valres = validate_password(newpass_decoded, newpass1); 

  switch (valres)
  {
    case VALRES_OK:
      break;
    case VALRES_TOO_SHORT:
      return get_edit_user_internal(NULL, USER_FORM_ERR_PASSWD_TOO_SHORT, req->client_info);
    case VALRES_TOO_LONG:
      return get_edit_user_internal(NULL, USER_FORM_ERR_PASSWD_TOO_LONG, req->client_info);
    case VALRES_UNMET:
      return get_edit_user_internal(NULL, USER_FORM_ERR_PASSWD_UNMET, req->client_info);
    case VALRES_INV_ENC:
      return get_edit_user_internal(NULL, USER_FORM_ERR_PASSWD_INV_ENC, req->client_info);
    default:
      log_crit("update_user_password(): invalid validation error for user password: %d", valres);
  }

  // update user password
  if (update_password(req->client_info->user_id, newpass_decoded) == -1)
  {
    return response_error(STAT500);
  }

  char uri[64];
  sprintf(uri, "/user?id=%s", req->client_info->user_id);

  // redirect to user page
  return response_redirect(uri);
}


struct response* update_post(const struct request* req)
{
  if (req->client_info == NULL)
  {
    return response_redirect("/login");
  }

  const char* post_id = get_map_value_str(req->content, "id");
  const char* post_body = get_map_value_str(req->content, "body");

  // check if post exists
  ks_hashmap* post_info;
  if ((post_info = query_post_by_id(post_id)) == NULL)
  {
    return response_error(STAT404);
  }

  // check if client owns this post
  const char* author_id = get_map_value_str(post_info, FIELD_POST_AUTHOR_ID);
  if (strcmp(author_id, req->client_info->user_id) != 0)
  {
    // permission denied
    return response_error(STAT400);
  }
  ks_hashmap_delete(post_info);

  // validate post body
  char body_decoded[POST_BODY_BUF_LEN];
  enum validation_result valres = validate_post_body(body_decoded, post_body);

  switch (valres)
  {
    case VALRES_OK:
      break;
    case VALRES_TOO_SHORT:
      return get_edit_post_internal(body_decoded, post_id, POST_FORM_ERR_BODY_TOO_SHORT, req->client_info);
    case VALRES_TOO_LONG:
      return get_edit_post_internal(body_decoded, post_id, POST_FORM_ERR_BODY_TOO_LONG, req->client_info);
    case VALRES_INV_ENC:
      return get_edit_post_internal(body_decoded, post_id, POST_FORM_ERR_BODY_INV_ENC, req->client_info);
    default:
      log_crit("update_post(): invalid validation error for post body: %d", valres);
  }

  // update post body
  if (sql_update_post_body(post_id, body_decoded) == -1)
  {
    return response_error(STAT500);
  }

  char uri[64];
  sprintf(uri, "/post?id=%s", post_id);

  // redirect to post page
  return response_redirect(uri);
}


struct response* update_comment(const struct request* req)
{
  if (req->client_info == NULL)
  {
    return response_redirect("/login");
  }

  const char* comment_id = get_map_value_str(req->content, "id");
  const char* comment_body = get_map_value_str(req->content, "body");

  // check if comment exists
  ks_hashmap* comment_info;
  if ((comment_info = query_comment_by_id(comment_id)) == NULL)
  {
    return response_error(STAT404);
  }

  // check if client owns this comment
  const char* author_id = get_map_value_str(comment_info, FIELD_COMMENT_AUTHOR_ID);
  if (strcmp(author_id, req->client_info->user_id) != 0)
  {
    return response_error(STAT403);
  }

  // validate comment body
  char body_decoded[COMMENT_BODY_BUF_LEN];
  enum validation_result valres = validate_comment_body(body_decoded, comment_body);

  switch (valres)
  {
    case VALRES_OK:
      break;
    case VALRES_TOO_SHORT:
      return get_edit_comment_internal(body_decoded, comment_id, COMMENT_FORM_ERR_TOO_SHORT, req->client_info);
    case VALRES_TOO_LONG:
      return get_edit_comment_internal(body_decoded, comment_id, COMMENT_FORM_ERR_TOO_LONG, req->client_info);
    case VALRES_INV_ENC:
      return get_edit_comment_internal(body_decoded, comment_id, COMMENT_FORM_ERR_INV_ENC, req->client_info);
    default:
      log_crit("update_comment(): invalid validation error for comment body: %d", valres);
  }

  // update comment body
  if (sql_update_comment_body(comment_id, body_decoded) == -1)
  {
    ks_hashmap_delete(comment_info);
    return response_error(STAT500);
  }

  const char* post_id = get_map_value_str(comment_info, FIELD_COMMENT_POST_ID);
  char uri[64];
  sprintf(uri, "/post?id=%s#%s", post_id, comment_id);
  ks_hashmap_delete(comment_info);

  return response_redirect(uri);
}


struct response* update_community_about(const struct request* req)
{
  if (req->client_info == NULL)
  {
    return response_redirect("/login");
  }

  const char* community_id = get_map_value_str(req->content, "id");
  const char* community_about = get_map_value_str(req->content, "about");

  // check if community exists
  ks_hashmap* community_info;
  if ((community_info = query_community_by_id(community_id)) == NULL)
  {
    return response_error(STAT404);
  }

  // check if client owns this community
  const char* owner_id = get_map_value_str(community_info, FIELD_COMMUNITY_OWNER_ID);
  if (strcmp(owner_id, req->client_info->user_id) != 0)
  {
    // check if client is a mod
    ks_hashmap* mod_info;
    if ((mod_info = query_moderator_by_community_id_user_id(community_id, req->client_info->user_id)) == NULL)
    {
      // check if client is an admin
      ks_hashmap* admin_info;
      if ((admin_info = query_administrator_by_user_id(req->client_info->user_id)) == NULL)
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
  enum validation_result valres = validate_community_about(about_decoded, community_about);

  switch (valres)
  {
    case VALRES_OK:
      break;
    case VALRES_TOO_SHORT:
      return get_edit_community_internal(about_decoded, community_id, COMMUNITY_FORM_ERR_ABOUT_TOO_SHORT, req->client_info);
    case VALRES_TOO_LONG:
      return get_edit_community_internal(about_decoded, community_id, COMMUNITY_FORM_ERR_ABOUT_TOO_LONG, req->client_info);
    case VALRES_INV_ENC:
      return get_edit_community_internal(about_decoded, community_id, COMMUNITY_FORM_ERR_ABOUT_INV_ENC, req->client_info);
    default:
      log_crit("update_community_about(): invalid validation error for community about: %d", valres);
  }

  // update community about
  if (sql_update_community_about(community_id, about_decoded) == -1)
  {
    return response_error(STAT500);
  }

  char uri[64];
  sprintf(uri, "/community?id=%s", community_id);

  return response_redirect(uri);
}
