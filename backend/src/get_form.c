#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <kylestructs.h>

#include <string_map.h>
#include <response.h>
#include <server.h>
#include <log_manager.h>
#include <auth_manager.h>
#include <templating.h>
#include <http_get.h>
#include <sql_manager.h>
#include <get_form.h>


struct response* get_login(const struct request* req)
{
  return get_login_internal(NULL, USER_FORM_ERR_NONE, req->client_info);
}


struct response* get_login_internal(const char* submitted_user_name, enum user_form_error err, const struct auth_token* client_info)
{
  ks_hashmap* page_data = ks_hashmap_new(KS_CHARP, 8);

  // only get actual login html if user is not logged in yet
  if (client_info == NULL)
  {
    add_map_value_str(page_data, TEMPLATE_PATH_KEY, HTML_LOGIN);

    if (err == USER_FORM_ERR_BAD_LOGIN)
    {
      if (submitted_user_name != NULL)
      {
        add_map_value_str(page_data, SUBMITTED_LOGIN_UNAME_KEY, submitted_user_name);
      }
      add_map_value_str(page_data, LOGIN_ERR_KEY, "Bad login");
    }

    else if (err != USER_FORM_ERR_NONE)
    {
      if (submitted_user_name != NULL)
      {
        add_map_value_str(page_data, SUBMITTED_SIGNUP_UNAME_KEY, submitted_user_name);
      }
      switch (err)
      {
        case USER_FORM_ERR_UNAME_TAKEN:
          add_map_value_str(page_data, SIGNUP_ERR_KEY, "Username already exists");
          break;
        case USER_FORM_ERR_UNAME_TOO_SHORT:
          add_map_value_str(page_data, SIGNUP_ERR_KEY, "Username too short");
          break;
        case USER_FORM_ERR_UNAME_TOO_LONG:
          add_map_value_str(page_data, SIGNUP_ERR_KEY, "Username too long");
          break;
        case USER_FORM_ERR_UNAME_INV_CHAR:
          add_map_value_str(page_data, SIGNUP_ERR_KEY, "Username contains invalid characters");
          break;
        case USER_FORM_ERR_PASSWD_MISMATCH:
          add_map_value_str(page_data, SIGNUP_ERR_KEY, "Password inputs do not match");
          break;
        case USER_FORM_ERR_PASSWD_TOO_SHORT:
          add_map_value_str(page_data, SIGNUP_ERR_KEY, "Password too short");
          break;
        case USER_FORM_ERR_PASSWD_TOO_LONG:
          add_map_value_str(page_data, SIGNUP_ERR_KEY, "Password too long");
          break;
        case USER_FORM_ERR_PASSWD_UNMET:
          add_map_value_str(page_data, SIGNUP_ERR_KEY, "Password does not meet character requirement");
          break;
        case USER_FORM_ERR_PASSWD_INV_ENC:
          add_map_value_str(page_data, SIGNUP_ERR_KEY, "Password contains invalid character encoding");
          break;
        case USER_FORM_ERR_ABOUT_TOO_SHORT:
          add_map_value_str(page_data, SIGNUP_ERR_KEY, "User about too short");
          break;
        case USER_FORM_ERR_ABOUT_TOO_LONG:
          add_map_value_str(page_data, SIGNUP_ERR_KEY, "User about too long");
          break;
        case USER_FORM_ERR_ABOUT_INV_ENC:
          add_map_value_str(page_data, SIGNUP_ERR_KEY, "User about contains invalid character encoding");
          break;
        default:
          log_crit("get_login(): invalid login error no.");
      }
    }
  }
  else
  {
    add_map_value_str(page_data, TEMPLATE_PATH_KEY, HTML_ALREADY_LOGGED_IN);
  }

  // wrap page data
  page_data = wrap_page_data(client_info, page_data);

  // build content 
  char* content;
  if ((content = build_template(page_data)) == NULL)
  {
    ks_hashmap_delete(page_data);
    return response_error(STAT500);
  }
  ks_hashmap_delete(page_data);

  // build and return response
  return response_ok(content);
}


struct response* get_edit_user(const struct request* req)
{
  return get_edit_user_internal(NULL, USER_FORM_ERR_NONE, req->client_info);
}


struct response* get_edit_user_internal(const char* submitted_about, enum user_form_error err, const struct auth_token* client_info)
{
  if (client_info == NULL)
  {
    return response_redirect("./login");
  }

  // get user info
  ks_hashmap* page_data;
  if ((page_data = query_user_by_name(client_info->user_name)) == NULL)
  {
    // this should not happen if client_info is not NULL
    return response_error(STAT404);
  }
  add_map_value_str(page_data, TEMPLATE_PATH_KEY, HTML_EDIT_USER);

  if (err == USER_FORM_ERR_NONE)
  {
    const char* existing_about = get_map_value_str(page_data, FIELD_USER_ABOUT);
    add_map_value_str(page_data, SUBMITTED_USER_ABOUT_KEY, existing_about);
  }
  else
  {
    if (submitted_about != NULL)
    {
      add_map_value_str(page_data, SUBMITTED_USER_ABOUT_KEY, submitted_about);
    }
    switch (err)
    {
      case USER_FORM_ERR_BAD_LOGIN:
        add_map_value_str(page_data, EDIT_USER_PASSWD_ERR_KEY, "Wrong password");
        break;
      case USER_FORM_ERR_PASSWD_MISMATCH:
        add_map_value_str(page_data, EDIT_USER_PASSWD_ERR_KEY, "New password inputs do not match");
        break;
      case USER_FORM_ERR_PASSWD_TOO_SHORT:
        add_map_value_str(page_data, EDIT_USER_PASSWD_ERR_KEY, "New password too short");
        break;
      case USER_FORM_ERR_PASSWD_TOO_LONG:
        add_map_value_str(page_data, EDIT_USER_PASSWD_ERR_KEY, "New password too long");
        break;
      case USER_FORM_ERR_PASSWD_INV_ENC:
        add_map_value_str(page_data, EDIT_USER_PASSWD_ERR_KEY, "New password contains invalid encoding");
        break;
      case USER_FORM_ERR_ABOUT_TOO_SHORT:
        add_map_value_str(page_data, EDIT_USER_ABOUT_ERR_KEY, "User about too short");
        break;
      case USER_FORM_ERR_ABOUT_TOO_LONG:
        add_map_value_str(page_data, EDIT_USER_ABOUT_ERR_KEY, "User about too long");
        break;
      case USER_FORM_ERR_ABOUT_INV_ENC:
        add_map_value_str(page_data, EDIT_USER_ABOUT_ERR_KEY, "User about contains invalid encoding");
        break;
      default:
        log_crit("get_edit_user(): invalid edit user form error: %d", err); 
    }
  }

  // wrap page data
  page_data = wrap_page_data(client_info, page_data);

  // build content
  char* content;
  if ((content = build_template(page_data)) == NULL)
  {
    ks_hashmap_delete(page_data);
    return response_error(STAT500);
  }
  ks_hashmap_delete(page_data);

  // build and return response
  return response_ok(content);
}


struct response* get_new_post(const struct request* req)
{
  const char* community_id = get_map_value_str(req->query, "id");

  return get_new_post_internal(NULL, NULL, community_id, POST_FORM_ERR_NONE, req->client_info);
}


struct response* get_new_post_internal(const char* submitted_title, const char* submitted_body, const char* community_id, enum post_form_error err, const struct auth_token* client_info)
{
  if (client_info == NULL)
  {
    return response_redirect("/login");
  }

  // get community info
  ks_hashmap* page_data;
  if (community_id == NULL || (page_data = query_community_by_id(community_id)) == NULL)
  {
    return response_error(STAT404);
  }
  add_map_value_str(page_data, TEMPLATE_PATH_KEY, HTML_NEW_POST);

  // copy previously submitted data into page if present
  if (submitted_title != NULL)
  {
    add_map_value_str(page_data, SUBMITTED_POST_TITLE_KEY, submitted_title);
  }

  if (submitted_body != NULL)
  {
    add_map_value_str(page_data, SUBMITTED_POST_BODY_KEY, submitted_body);
  }

  switch (err)
  {
    case POST_FORM_ERR_NONE:
      break;
    case POST_FORM_ERR_TITLE_TOO_SHORT:
      add_map_value_str(page_data, POST_FORM_ERR_KEY, "Title too short");
      break;
    case POST_FORM_ERR_TITLE_TOO_LONG:
      add_map_value_str(page_data, POST_FORM_ERR_KEY, "Title too long");
      break;
    case POST_FORM_ERR_TITLE_INV_ENC:
      add_map_value_str(page_data, POST_FORM_ERR_KEY, "Title contains invalid encoding");
      break;
    case POST_FORM_ERR_BODY_TOO_SHORT:
      add_map_value_str(page_data, POST_FORM_ERR_KEY, "Body too short");
      break;
    case POST_FORM_ERR_BODY_TOO_LONG:
      add_map_value_str(page_data, POST_FORM_ERR_KEY, "Body too long");
      break;
    case POST_FORM_ERR_BODY_INV_ENC:
      add_map_value_str(page_data, POST_FORM_ERR_KEY, "Body contains invalid encoding");
      break;
    default:
      log_crit("get_login(): invalid new post form error: %d", err);
  }

  // wrap page data
  page_data = wrap_page_data(client_info, page_data);

  // build content
  char* content;
  if ((content = build_template(page_data)) == NULL)
  {
    ks_hashmap_delete(page_data);
    return response_error(STAT500);
  }
  ks_hashmap_delete(page_data);

  // build and return response
  return response_ok(content);
}


struct response* get_edit_post(const struct request* req)
{
  const char* post_id = get_map_value_str(req->query, "id");

  return get_edit_post_internal(NULL, post_id, POST_FORM_ERR_NONE, req->client_info);
}


struct response* get_edit_post_internal(const char* submitted_body, const char* post_id, enum post_form_error err, const struct auth_token* client_info)
{
  if (client_info == NULL)
  {
    return response_redirect("/login");
  }

  // get post info
  ks_hashmap* page_data;
  if (post_id == NULL || (page_data = query_post_by_id(post_id)) == NULL)
  {
    return response_error(STAT404);
  }

  // check if community has been deleted
  const char* community_id = get_map_value_str(page_data, FIELD_POST_COMMUNITY_ID);
  if (strcmp(community_id, "1") == 0)
  {
    ks_hashmap_delete(page_data);
    return response_error(STAT403);
  }

  // make sure client is the author
  const char* author_id = get_map_value_str(page_data, FIELD_POST_AUTHOR_ID);
  if (strcmp(client_info->user_id, author_id) != 0)
  {
    ks_hashmap_delete(page_data);
    return response_error(STAT403);
  }
  add_map_value_str(page_data, TEMPLATE_PATH_KEY, HTML_EDIT_POST);

  // copy previously submitted post body, or else existing post body
  if (submitted_body != NULL)
  {
    add_map_value_str(page_data, SUBMITTED_POST_BODY_KEY, submitted_body);
  }
  else
  {
    const char* existing_body = get_map_value_str(page_data, FIELD_POST_BODY);
    add_map_value_str(page_data, SUBMITTED_POST_BODY_KEY, existing_body);
  }

  // you can only edit the post body, not the title. Why? just cuz
  // thats how reddit does it.
  switch (err)
  {
    case POST_FORM_ERR_NONE:
      break;
    case POST_FORM_ERR_BODY_TOO_SHORT:
      add_map_value_str(page_data, EDIT_POST_FORM_ERR_KEY, "Post body too short");
      break;
    case POST_FORM_ERR_BODY_TOO_LONG:
      add_map_value_str(page_data, EDIT_POST_FORM_ERR_KEY, "Post body too long");
      break;
    case POST_FORM_ERR_BODY_INV_ENC:
      add_map_value_str(page_data, EDIT_POST_FORM_ERR_KEY, "Post body contains invalid encoding");
      break;
    default:
      log_crit("get_edit_post(): invalid edit post form error %d", err);
  }

  // wrap page data
  page_data = wrap_page_data(client_info, page_data);

  // build content
  char* content;
  if ((content = build_template(page_data)) == NULL)
  {
    ks_hashmap_delete(page_data);
    return response_error(STAT500);
  }
  ks_hashmap_delete(page_data);

  // build and return response
  return response_ok(content);
}


struct response* get_new_comment(const struct request* req)
{
  const char* post_id = get_map_value_str(req->query, "id");

  return get_new_comment_internal(NULL, post_id, COMMENT_FORM_ERR_NONE, req->client_info);
}


struct response* get_new_comment_internal(const char* submitted_body, const char* post_id, enum comment_form_error err, const struct auth_token* client_info)
{
  if (client_info == NULL)
  {
    return response_redirect("/login");
  }

  // get post info
  ks_hashmap* page_data;
  if (post_id == NULL || (page_data = query_post_by_id(post_id)) == NULL)
  {
    return response_error(STAT404);
  }
  add_map_value_str(page_data, TEMPLATE_PATH_KEY, HTML_NEW_COMMENT);

  if (submitted_body != NULL)
  {
    add_map_value_str(page_data, SUBMITTED_COMMENT_BODY_KEY, submitted_body);
  }

  switch (err)
  {
    case COMMENT_FORM_ERR_NONE:
      break;
    case COMMENT_FORM_ERR_TOO_SHORT:
      add_map_value_str(page_data, COMMENT_FORM_ERR_KEY, "Comment body too short");
      break;
    case COMMENT_FORM_ERR_TOO_LONG:
      add_map_value_str(page_data, COMMENT_FORM_ERR_KEY, "Comment body too long");
      break;
    case COMMENT_FORM_ERR_INV_ENC:
      add_map_value_str(page_data, COMMENT_FORM_ERR_KEY, "Comment body contains invalid character encoding");
      break;
    default:
      log_crit("get_new_comment(): invalid new comment form error: %d", err);
  }

  // put page data together
  page_data = wrap_page_data(client_info, page_data);

  // build content
  char* content;
  if ((content = build_template(page_data)) == NULL)
  {
    ks_hashmap_delete(page_data);
    return response_error(STAT500);
  }
  ks_hashmap_delete(page_data);

  // build and return response
  return response_ok(content);
}


struct response* get_edit_comment(const struct request* req)
{
  const char* comment_id = get_map_value_str(req->query, "id");

  return get_edit_comment_internal(NULL, comment_id, COMMENT_FORM_ERR_NONE, req->client_info);
}


struct response* get_edit_comment_internal(const char* submitted_body, const char* comment_id, enum comment_form_error err, const struct auth_token* client_info)
{
  if (client_info == NULL)
  {
    return response_redirect("/login");
  }

  // get comment info
  ks_hashmap* comment_info;
  if (comment_id == NULL || (comment_info = query_comment_by_id(comment_id)) == NULL)
  {
    return response_error(STAT404);
  }

  // get post info
  const char* post_id = get_map_value_str(comment_info, FIELD_COMMENT_POST_ID);
  ks_hashmap* page_data;
  if ((page_data = query_post_by_id(post_id)) == NULL)
  {
    ks_hashmap_delete(comment_info);
    // if a comment's parent post is missing, something went wrong
    return response_error(STAT500);
  }

  // check if community has been deleted
  const char* community_id = get_map_value_str(comment_info, FIELD_COMMENT_COMMUNITY_ID);
  if (strcmp(community_id, "1") == 0)
  {
    ks_hashmap_delete(comment_info);
    ks_hashmap_delete(page_data);
    return response_error(STAT403);
  }

  // check if post has been deleted
  const char* post_author_id = get_map_value_str(page_data, FIELD_POST_AUTHOR_ID);
  if (strcmp(post_author_id, "1") == 0)
  {
    ks_hashmap_delete(comment_info);
    ks_hashmap_delete(page_data);
    return response_error(STAT403);
  }

  // make sure client is the author
  const char* author_id = get_map_value_str(comment_info, FIELD_COMMENT_AUTHOR_ID);
  if (strcmp(client_info->user_id, author_id) != 0)
  {
    ks_hashmap_delete(comment_info);
    ks_hashmap_delete(page_data);
    return response_error(STAT403);
  }

  // add html template path
  add_map_value_str(page_data, TEMPLATE_PATH_KEY, HTML_EDIT_COMMENT);

  // add comment ID to page data
  add_map_value_str(page_data, FIELD_COMMENT_ID, comment_id);

  // copy previously submitted comment body, or else existing comment body
  if (submitted_body != NULL)
  {
    add_map_value_str(page_data, SUBMITTED_COMMENT_BODY_KEY, submitted_body);
  }
  else
  {
    const char* existing_body = get_map_value_str(comment_info, FIELD_COMMENT_BODY);
    add_map_value_str(page_data, SUBMITTED_COMMENT_BODY_KEY, existing_body);
  }
  ks_hashmap_delete(comment_info);

  switch (err)
  {
    case COMMENT_FORM_ERR_NONE:
      break;
    case COMMENT_FORM_ERR_TOO_SHORT:
      add_map_value_str(page_data, EDIT_COMMENT_FORM_ERR_KEY, "Comment body too short");
      break;
    case COMMENT_FORM_ERR_TOO_LONG:
      add_map_value_str(page_data, EDIT_COMMENT_FORM_ERR_KEY, "Comment body too long");
      break;
    case COMMENT_FORM_ERR_INV_ENC:
      add_map_value_str(page_data, EDIT_COMMENT_FORM_ERR_KEY, "Comment body contains invalid character encoding");
      break;
    default:
      log_crit("get_edit_comment(): invalid edit comment form error: %d", err);
  }

  // put page data together
  page_data = wrap_page_data(client_info, page_data);

  // build content
  char* content;
  if ((content = build_template(page_data)) == NULL)
  {
    ks_hashmap_delete(page_data);
    return response_error(STAT500);
  }
  ks_hashmap_delete(page_data);

  // build and return response
  return response_ok(content);
}


struct response* get_new_community(const struct request* req)
{
  return get_new_community_internal(NULL, NULL, COMMUNITY_FORM_ERR_NONE, req->client_info);
}


struct response* get_new_community_internal(const char* submitted_name, const char* submitted_about, enum community_form_error err, const struct auth_token* client_info)
{
  if (client_info == NULL)
  {
    return response_redirect("/login");
  }

  ks_hashmap* page_data = ks_hashmap_new(KS_CHARP, 1);
  add_map_value_str(page_data, TEMPLATE_PATH_KEY, HTML_NEW_COMMUNITY);

  if (submitted_name != NULL)
  {
    add_map_value_str(page_data, SUBMITTED_COMMUNITY_NAME_KEY, submitted_name);
  }

  if (submitted_about != NULL)
  {
    add_map_value_str(page_data, SUBMITTED_COMMUNITY_ABOUT_KEY, submitted_about);
  }

  switch (err)
  {
    case COMMUNITY_FORM_ERR_NONE:
      break;
    case COMMUNITY_FORM_ERR_NAME_TOO_SHORT:
      add_map_value_str(page_data, COMMUNITY_FORM_ERR_KEY, "Community name too short");
      break;
    case COMMUNITY_FORM_ERR_NAME_TOO_LONG:
      add_map_value_str(page_data, COMMUNITY_FORM_ERR_KEY, "Community name too long");
      break;
    case COMMUNITY_FORM_ERR_NAME_INV_CHAR:
      add_map_value_str(page_data, COMMUNITY_FORM_ERR_KEY, "Community name contains invalid character");
      break;
    case COMMUNITY_FORM_ERR_NAME_ALREADY_EXISTS:
      add_map_value_str(page_data, COMMUNITY_FORM_ERR_KEY, "Community name already exists");
      break;
    case COMMUNITY_FORM_ERR_ABOUT_TOO_SHORT:
      add_map_value_str(page_data, COMMUNITY_FORM_ERR_KEY, "Community about too short");
      break;
    case COMMUNITY_FORM_ERR_ABOUT_TOO_LONG:
      add_map_value_str(page_data, COMMUNITY_FORM_ERR_KEY, "Community about too long");
      break;
    case COMMUNITY_FORM_ERR_ABOUT_INV_ENC:
      add_map_value_str(page_data, COMMUNITY_FORM_ERR_KEY, "Community about contains invalid character encoding");
      break;
    default:
      log_crit("get_new_community(): invalid community form error: %d", err);
  }

  // put page data together
  page_data = wrap_page_data(client_info, page_data);

  // build content
  char* content;
  if ((content = build_template(page_data)) == NULL)
  {
    ks_hashmap_delete(page_data);
    return response_error(STAT500);
  }
  ks_hashmap_delete(page_data);

  // build and return response
  return response_ok(content);
}


struct response* get_edit_community(const struct request* req)
{
  const char* community_id = get_map_value_str(req->query, "id");

  return get_edit_community_internal(NULL, community_id, COMMUNITY_FORM_ERR_NONE, req->client_info);
}


struct response* get_edit_community_internal(const char* submitted_about, const char* community_id, enum community_form_error err, const struct auth_token* client_info)
{
  if (client_info == NULL)
  {
    return response_redirect("/login");
  }

  // get community info
  ks_hashmap* page_data;
  if (community_id == NULL || (page_data = query_community_by_id(community_id)) == NULL)
  {
    return response_error(STAT404);
  }

  // check if client is owner
  const char* owner_id = get_map_value_str(page_data, FIELD_COMMUNITY_OWNER_ID);
  if (strcmp(client_info->user_id, owner_id) != 0)
  {
    // check if client is moderator of this community
    ks_hashmap* mod_info;
    if ((mod_info = query_moderator_by_user_id_community_id(client_info->user_id, community_id)) == NULL)
    {
      ks_hashmap_delete(page_data);
      return response_error(STAT403);
    }
    else ks_hashmap_delete(mod_info);
  }
  add_map_value_str(page_data, TEMPLATE_PATH_KEY, HTML_EDIT_COMMUNITY);

  // copy submitted community about if present, else existing community about
  if (submitted_about != NULL)
  {
    add_map_value_str(page_data, SUBMITTED_COMMUNITY_ABOUT_KEY, submitted_about);
  }
  else
  {
    const char* existing_about = get_map_value_str(page_data, FIELD_COMMUNITY_ABOUT);
    add_map_value_str(page_data, SUBMITTED_COMMUNITY_ABOUT_KEY, existing_about);
  }

  switch (err)
  {
    case COMMUNITY_FORM_ERR_NONE:
      break;
    case COMMUNITY_FORM_ERR_ABOUT_TOO_SHORT:
      add_map_value_str(page_data, EDIT_COMMUNITY_FORM_ERR_KEY, "Community about too short");
      break;
    case COMMUNITY_FORM_ERR_ABOUT_TOO_LONG:
      add_map_value_str(page_data, EDIT_COMMUNITY_FORM_ERR_KEY, "Community about too long");
      break;
    case COMMUNITY_FORM_ERR_ABOUT_INV_ENC:
      add_map_value_str(page_data, EDIT_COMMUNITY_FORM_ERR_KEY, "Community about contains invalid character encoding");
      break;
    case COMMUNITY_FORM_ERR_MOD_USER_NOT_EXIST:
      add_map_value_str(page_data, NEW_MOD_FORM_ERR_KEY, "User not found");
      break;
    case COMMUNITY_FORM_ERR_MOD_USER_ALREADY:
      add_map_value_str(page_data, NEW_MOD_FORM_ERR_KEY, "User is already a moderator of this community");
      break;
    case COMMUNITY_FORM_ERR_MOD_OWNER_ONLY:
      add_map_value_str(page_data, NEW_MOD_FORM_ERR_KEY, "Only the community owner can add moderators");
      break;
    default:
      log_crit("get_new_community(): invalid community form error: %d", err);
  }

  // put page data together
  page_data = wrap_page_data(client_info, page_data);

  // build content
  char* content;
  if ((content = build_template(page_data)) == NULL)
  {
    ks_hashmap_delete(page_data);
    return response_error(STAT500);
  }
  ks_hashmap_delete(page_data);

  // build and return response
  return response_ok(content);
}
