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


struct response* get_login(const struct auth_token* client_info, enum login_error err)
{
  ks_hashmap* page_data = ks_hashmap_new(KS_CHARP, 8);

  // only get actual login html if user is not logged in yet
  if (client_info == NULL)
  {
    add_map_value_str(page_data, TEMPLATE_PATH_KEY, HTML_LOGIN);

    // check for login/signup error
    if (err == LOGINERR_BAD_LOGIN)
    {
      add_map_value_str(page_data, SIGNUP_ERROR_KEY, "");
      add_map_value_str(page_data, LOGIN_ERROR_KEY, BAD_LOGIN_MSG);
    }
    else if (err == LOGINERR_UNAME_TAKEN)
    {
      add_map_value_str(page_data, SIGNUP_ERROR_KEY, UNAME_TAKEN_MSG);
      add_map_value_str(page_data, LOGIN_ERROR_KEY, "");
    }
    else if (err == LOGINERR_EMPTY)
    {
      add_map_value_str(page_data, SIGNUP_ERROR_KEY, EMPTY_INPUT_MSG);
      add_map_value_str(page_data, LOGIN_ERROR_KEY, "");
    }
    else if (err == LOGINERR_NONE)
    {
      add_map_value_str(page_data, SIGNUP_ERROR_KEY, "");
      add_map_value_str(page_data, LOGIN_ERROR_KEY, "");
    }
    else
    {
      log_crit("get_login(): invalid login error no.");
    }
  }
  else
  {
    add_map_value_str(page_data, TEMPLATE_PATH_KEY, HTML_ALREADY_LOGGED_IN);
  }

  // wrap page data
  page_data = wrap_page_data(client_info, page_data, CSS_LOGIN, "");

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


struct response* get_edit_user(const struct auth_token* client_info)
{
  if (client_info == NULL)
  {
    return response_redirect("./login");
  }

  // get user info
  ks_hashmap* page_data;
  if ((page_data = get_user_info(client_info->user_name)) == NULL)
  { 
    return response_error(STAT404);
  }
  add_map_value_str(page_data, TEMPLATE_PATH_KEY, HTML_EDIT_USER);

  // wrap page data
  page_data = wrap_page_data(client_info, page_data, CSS_FORM, "");

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


struct response* get_new_post(const char* community_name, const struct auth_token* client_info)
{
  if (client_info == NULL)
  {
    return response_redirect("/login");
  }

  // get community info
  ks_hashmap* page_data;
  if (community_name == NULL || (page_data = get_community_info(community_name)) == NULL)
  {
    return response_error(STAT404);
  }
  add_map_value_str(page_data, TEMPLATE_PATH_KEY, HTML_NEW_POST);

  // wrap page data
  page_data = wrap_page_data(client_info, page_data, CSS_FORM, "");

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


struct response* get_edit_post(const char* post_id, const struct auth_token* client_info)
{
  if (client_info == NULL)
  {
    return response_redirect("/login");
  }

  // get post info
  ks_hashmap* page_data;
  if (post_id == NULL || (page_data = get_post_info(post_id)) == NULL)
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

  // wrap page data
  page_data = wrap_page_data(client_info, page_data, CSS_FORM, "");

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


struct response* get_new_comment(const char* post_id, const struct auth_token* client_info)
{
  if (client_info == NULL)
  {
    return response_redirect("/login");
  }

  // get post info
  ks_hashmap* page_data;
  if (post_id == NULL || (page_data = get_post_info(post_id)) == NULL)
  {
    return response_error(STAT404);
  }
  add_map_value_str(page_data, TEMPLATE_PATH_KEY, HTML_NEW_COMMENT);

  // put page data together
  page_data = wrap_page_data(client_info, page_data, CSS_FORM, "");

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


struct response* get_edit_comment(const char* comment_id, const struct auth_token* client_info)
{
  if (client_info == NULL)
  {
    return response_redirect("/login");
  }

  // get comment info
  ks_hashmap* page_data;
  if (comment_id == NULL || (page_data = get_comment_info(comment_id)) == NULL)
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

  // put page data together
  page_data = wrap_page_data(client_info, page_data, CSS_FORM, "");

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


struct response* get_new_community(const struct auth_token* client_info)
{
  if (client_info == NULL)
  {
    return response_redirect("/login");
  }

  ks_hashmap* page_data = ks_hashmap_new(KS_CHARP, 1);
  add_map_value_str(page_data, TEMPLATE_PATH_KEY, HTML_NEW_COMMUNITY);

  // put page data together
  page_data = wrap_page_data(client_info, page_data, CSS_FORM, "");

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


struct response* get_edit_community(const char* community_id, const struct auth_token* client_info)
{
  if (client_info == NULL)
  {
    return response_redirect("/login");
  }

  // get community info
  ks_hashmap* page_data;
  if (community_id == NULL || (page_data = get_community_info(community_id)) == NULL)
  {
    return response_error(STAT404);
  }

  // check if client is owner
  const char* owner_id = get_map_value_str(page_data, FIELD_COMMUNITY_OWNER_ID);
  if (strcmp(client_info->user_id, owner_id) != 0)
  {
    // check if client is moderator of this community
    ks_hashmap* mod_info;
    if ((mod_info = get_moderator_info(community_id, client_info->user_id)) == NULL)
    {
      // check if client is an admin
      ks_hashmap* admin_info;
      if ((admin_info = get_administrator_info(client_info->user_id)) == NULL)
      {
        ks_hashmap_delete(page_data);
        return response_error(STAT403);
      }
      else ks_hashmap_delete(admin_info);
    }
    else ks_hashmap_delete(mod_info);
  }
  add_map_value_str(page_data, TEMPLATE_PATH_KEY, HTML_EDIT_COMMUNITY);

  // put page data together
  page_data = wrap_page_data(client_info, page_data, CSS_FORM, "");

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
