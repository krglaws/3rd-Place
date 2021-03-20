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


struct response* get_login(const char* submitted_user_name, enum user_form_error err, const struct auth_token* client_info)
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
      add_map_value_str(page_data, LOGIN_ERR_KEY, "<p>Bad login</p>");
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
          add_map_value_str(page_data, SIGNUP_ERR_KEY, "<p>Username already exists</p>");
          break;
        case USER_FORM_ERR_UNAME_TOO_SHORT:
          add_map_value_str(page_data, SIGNUP_ERR_KEY, "<p>Username too short</p>");
          break;
        case USER_FORM_ERR_UNAME_TOO_LONG:
          add_map_value_str(page_data, SIGNUP_ERR_KEY, "<p>Username too long</p>");
          break;
        case USER_FORM_ERR_UNAME_INV_CHAR:
          add_map_value_str(page_data, SIGNUP_ERR_KEY, "<p>Username contains invalid characters</p>");
          break;
        case USER_FORM_ERR_PASSWD_MISMATCH:
          add_map_value_str(page_data, EDIT_PASSWD_ERR_KEY, "<p>Password inputs do not match</p>");
          break;
        case USER_FORM_ERR_PASSWD_TOO_SHORT:
          add_map_value_str(page_data, SIGNUP_ERR_KEY, "<p>Password too short</p>");
          break;
        case USER_FORM_ERR_PASSWD_TOO_LONG:
          add_map_value_str(page_data, SIGNUP_ERR_KEY, "<p>Password too long</p>");
          break;
        case USER_FORM_ERR_PASSWD_UNMET:
          add_map_value_str(page_data, SIGNUP_ERR_KEY, "<p>Password does not meet character requirement</p>");
          break;
        case USER_FORM_ERR_PASSWD_INV_ENC:
          add_map_value_str(page_data, SIGNUP_ERR_KEY, "<p>Password contains invalid character encoding</p>");
          break;
        case USER_FORM_ERR_ABOUT_TOO_SHORT:
          add_map_value_str(page_data, SIGNUP_ERR_KEY, "<p>User about too short</p>");
          break;
        case USER_FORM_ERR_ABOUT_TOO_LONG:
          add_map_value_str(page_data, SIGNUP_ERR_KEY, "<p>User about too long</p>");
          break;
        case USER_FORM_ERR_ABOUT_INV_ENC:
          add_map_value_str(page_data, SIGNUP_ERR_KEY, "<p>User about contains invalid character encoding</p>");
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
  page_data = wrap_page_data(client_info, page_data, CSS_LOGIN, NULL);

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


struct response* get_edit_user(const char* submitted_about, enum user_form_error err, const struct auth_token* client_info)
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
        add_map_value_str(page_data, EDIT_PASSWD_ERR_KEY, "<p>Wrong password</p>");
        break;
      case USER_FORM_ERR_PASSWD_MISMATCH:
        add_map_value_str(page_data, EDIT_PASSWD_ERR_KEY, "<p>New password inputs do not match</p>");
        break;
      case USER_FORM_ERR_PASSWD_TOO_SHORT:
        add_map_value_str(page_data, EDIT_PASSWD_ERR_KEY, "<p>New password too short</p>");
        break;
      case USER_FORM_ERR_PASSWD_TOO_LONG:
        add_map_value_str(page_data, EDIT_PASSWD_ERR_KEY, "<p>New password too long</p>");
        break;
      case USER_FORM_ERR_PASSWD_INV_ENC:
        add_map_value_str(page_data, EDIT_PASSWD_ERR_KEY, "<p>New password contains invalid encoding</p>");
        break;
      case USER_FORM_ERR_ABOUT_TOO_SHORT:
        add_map_value_str(page_data, EDIT_ABOUT_ERR_KEY, "<p>User about too short</p>");
        break;
      case USER_FORM_ERR_ABOUT_TOO_LONG:
        add_map_value_str(page_data, EDIT_ABOUT_ERR_KEY, "<p>User about too long</p>");
        break;
      case USER_FORM_ERR_ABOUT_INV_ENC:
        add_map_value_str(page_data, EDIT_ABOUT_ERR_KEY, "<p>User about contains invalid encoding</p>");
        break;
      default:
        log_crit("get_edit_user(): invalid edit user form error: %d", err); 
    }
  }

  // wrap page data
  page_data = wrap_page_data(client_info, page_data, CSS_FORM, NULL);

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


struct response* get_new_post(const char* submitted_title, const char* submitted_body, const char* community_name, enum post_form_error err, const struct auth_token* client_info)
{
  if (client_info == NULL)
  {
    return response_redirect("/login");
  }

  // get community info
  ks_hashmap* page_data;
  if (community_name == NULL || (page_data = query_community_by_name(community_name)) == NULL)
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
      add_map_value_str(page_data, POST_FORM_ERR_KEY, "<p>Title too short</p>");
      break;
    case POST_FORM_ERR_TITLE_TOO_LONG:
      add_map_value_str(page_data, POST_FORM_ERR_KEY, "<p>Title too long</p>");
      break;
    case POST_FORM_ERR_TITLE_INV_ENC:
      add_map_value_str(page_data, POST_FORM_ERR_KEY, "<p>Title contains invalid encoding</p>");
      break;
    case POST_FORM_ERR_BODY_TOO_SHORT:
      add_map_value_str(page_data, POST_FORM_ERR_KEY, "<p>Body too short</p>");
      break;
    case POST_FORM_ERR_BODY_TOO_LONG:
      add_map_value_str(page_data, POST_FORM_ERR_KEY, "<p>Body too long</p>");
      break;
    case POST_FORM_ERR_BODY_INV_ENC:
      add_map_value_str(page_data, POST_FORM_ERR_KEY, "<p>Body contains invalid encoding</p>");
      break;
    default:
      log_crit("get_login(): invalid new post form error: %d", err);
  }

  // wrap page data
  page_data = wrap_page_data(client_info, page_data, CSS_FORM, NULL);

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


struct response* get_edit_post(const char* submitted_body, const char* post_id, enum post_form_error err, const struct auth_token* client_info)
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
      add_map_value_str(page_data, EDIT_POST_FORM_ERR_KEY, "<p>Post body too short</p>");
      break;
    case POST_FORM_ERR_BODY_TOO_LONG:
      add_map_value_str(page_data, EDIT_POST_FORM_ERR_KEY, "<p>Post body too long</p>");
      break;
    case POST_FORM_ERR_BODY_INV_ENC:
      add_map_value_str(page_data, EDIT_POST_FORM_ERR_KEY, "<p>Post body contains invalid encoding</p>");
      break;
    default:
      log_crit("get_edit_post(): invalid edit post form error %d", err);
  }

  // wrap page data
  page_data = wrap_page_data(client_info, page_data, CSS_FORM, NULL);

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


struct response* get_new_comment(const char* submitted_body, const char* post_id, enum comment_form_error err, const struct auth_token* client_info)
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
      add_map_value_str(page_data, COMMENT_FORM_ERR_KEY, "<p>Comment body too short</p>");
      break;
    case COMMENT_FORM_ERR_TOO_LONG:
      add_map_value_str(page_data, COMMENT_FORM_ERR_KEY, "<p>Comment body too long</p>");
      break;
    case COMMENT_FORM_ERR_INV_ENC:
      add_map_value_str(page_data, COMMENT_FORM_ERR_KEY, "<p>Comment body contains invalid character encoding</p>");
      break;
    default:
      log_crit("get_new_comment(): invalid new comment form error: %d", err);
  }

  // put page data together
  page_data = wrap_page_data(client_info, page_data, CSS_FORM, NULL);

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


struct response* get_edit_comment(const char* submitted_body, const char* comment_id, enum comment_form_error err, const struct auth_token* client_info)
{
  if (client_info == NULL)
  {
    return response_redirect("/login");
  }

  // get comment info
  ks_hashmap* page_data;
  if (comment_id == NULL || (page_data = query_comment_by_id(comment_id)) == NULL)
  {
    return response_error(STAT404);
  }

  // make sure client is the author
  const char* author_id = get_map_value_str(page_data, FIELD_COMMENT_AUTHOR_ID);
  if (strcmp(client_info->user_id, author_id) != 0)
  {
    ks_hashmap_delete(page_data);
    return response_error(STAT403);
  }
  add_map_value_str(page_data, TEMPLATE_PATH_KEY, HTML_NEW_COMMENT);

  // copy previously submitted comment body, or else existing comment body
  if (submitted_body != NULL)
  {
    add_map_value_str(page_data, SUBMITTED_COMMENT_BODY_KEY, submitted_body);
  }
  else
  {
    const char* existing_body = get_map_value_str(page_data, FIELD_COMMENT_BODY);
    add_map_value_str(page_data, SUBMITTED_COMMENT_BODY_KEY, existing_body);
  }

  switch (err)
  {
    case COMMENT_FORM_ERR_NONE:
      break;
    case COMMENT_FORM_ERR_TOO_SHORT:
      add_map_value_str(page_data, EDIT_COMMENT_FORM_ERR_KEY, "<p>Comment body too short</p>");
      break;
    case COMMENT_FORM_ERR_TOO_LONG:
      add_map_value_str(page_data, EDIT_COMMENT_FORM_ERR_KEY, "<p>Comment body too long</p>");
      break;
    case COMMENT_FORM_ERR_INV_ENC:
      add_map_value_str(page_data, EDIT_COMMENT_FORM_ERR_KEY, "<p>Comment body contains invalid character encoding</p>");
      break;
    default:
      log_crit("get_edit_comment(): invalid edit comment form error: %d", err);
  }

  // put page data together
  page_data = wrap_page_data(client_info, page_data, CSS_FORM, NULL);

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


struct response* get_new_community(const char* submitted_name, const char* submitted_about, enum community_form_error err, const struct auth_token* client_info)
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
      add_map_value_str(page_data, COMMUNITY_FORM_ERR_KEY, "<p>Community name too short</p>");
      break;
    case COMMUNITY_FORM_ERR_NAME_TOO_LONG:
      add_map_value_str(page_data, COMMUNITY_FORM_ERR_KEY, "<p>Community name too long</p>");
      break;
    case COMMUNITY_FORM_ERR_NAME_INV_CHAR:
      add_map_value_str(page_data, COMMUNITY_FORM_ERR_KEY, "<p>Community name contains invalid character</p>");
      break;
    case COMMUNITY_FORM_ERR_ABOUT_TOO_SHORT:
      add_map_value_str(page_data, COMMUNITY_FORM_ERR_KEY, "<p>Community about too short</p>");
      break;
    case COMMUNITY_FORM_ERR_ABOUT_TOO_LONG:
      add_map_value_str(page_data, COMMUNITY_FORM_ERR_KEY, "<p>Community about too long</p>");
      break;
    case COMMUNITY_FORM_ERR_ABOUT_INV_ENC:
      add_map_value_str(page_data, COMMUNITY_FORM_ERR_KEY, "<p>Community about contains invalid character encoding</p>");
      break;
    default:
      log_crit("get_new_community(): invalid community form error: %d", err);
  }

  // put page data together
  page_data = wrap_page_data(client_info, page_data, CSS_FORM, NULL);

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


struct response* get_edit_community(const char* submitted_about, const char* community_id, enum community_form_error err, const struct auth_token* client_info)
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
    if ((mod_info = query_moderator_by_community_id_user_id(community_id, client_info->user_id)) == NULL)
    {
      // check if client is an admin
      ks_hashmap* admin_info;
      if ((admin_info = query_administrator_by_user_id(client_info->user_id)) == NULL)
      {
        ks_hashmap_delete(page_data);
        return response_error(STAT403);
      }
      else ks_hashmap_delete(admin_info);
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
      add_map_value_str(page_data, EDIT_COMMUNITY_FORM_ERR_KEY, "<p>Community about too short</p>");
      break;
    case COMMUNITY_FORM_ERR_ABOUT_TOO_LONG:
      add_map_value_str(page_data, EDIT_COMMUNITY_FORM_ERR_KEY, "<p>Community about too long</p>");
      break;
    case COMMUNITY_FORM_ERR_ABOUT_INV_ENC:
      add_map_value_str(page_data, EDIT_COMMUNITY_FORM_ERR_KEY, "<p>Community about contains invalid character encoding</p>");
      break;
    default:
      log_crit("get_new_community(): invalid community form error: %d", err);
  }

  // put page data together
  page_data = wrap_page_data(client_info, page_data, CSS_FORM, NULL);

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
