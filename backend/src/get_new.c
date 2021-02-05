#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <kylestructs.h>

#include <string_map.h>
#include <common.h>
#include <log_manager.h>
#include <auth_manager.h>
#include <templating.h>
#include <http_get.h>
#include <sql_manager.h>
#include <get_new_post.h>


char* get_login(const struct auth_token* client_info, enum login_error err)
{
  ks_hashmap* page_content = ks_hashmap_new(KS_CHARP, 8);

  // only get actual login html if user is not logged in yet
  if (client_info == NULL)
  {
    add_map_value_str(page_content, TEMPLATE_PATH_KEY, HTML_LOGIN);

    // check for login/signup error
    if (err == LOGINERR_BAD_LOGIN)
    {
      add_map_value_str(page_content, SIGNUP_ERROR_KEY, " ");
      add_map_value_str(page_content, LOGIN_ERROR_KEY, BAD_LOGIN_MSG);
    }
    else if (err == LOGINERR_UNAME_TAKEN)
    {
      add_map_value_str(page_content, SIGNUP_ERROR_KEY, UNAME_TAKEN_MSG);
      add_map_value_str(page_content, LOGIN_ERROR_KEY, " ");
    }
    else if (err == LOGINERR_EMPTY)
    {
      add_map_value_str(page_content, SIGNUP_ERROR_KEY, EMPTY_INPUT_MSG);
      add_map_value_str(page_content, LOGIN_ERROR_KEY, " ");
    }
    else if (err == LOGINERR_NONE)
    {
      add_map_value_str(page_content, SIGNUP_ERROR_KEY, " ");
      add_map_value_str(page_content, LOGIN_ERROR_KEY, " ");
    }
    else
    {
      log_crit("get_login(): invalid login error no.");
    }
  }
  else
  {
    add_map_value_str(page_content, TEMPLATE_PATH_KEY, HTML_ALREADY_LOGGED_IN);
  }

  ks_hashmap* page_data = ks_hashmap_new(KS_CHARP, 8);
  add_map_value_str(page_data, TEMPLATE_PATH_KEY, HTML_MAIN);
  add_map_value_str(page_data, STYLE_PATH_KEY, CSS_LOGIN);
  add_map_value_str(page_data, SCRIPT_PATH_KEY, " ");
  add_map_value_hm(page_data, PAGE_CONTENT_KEY, page_content);
  add_nav_info(page_data, client_info);

  char* html = build_template(page_data);
  ks_hashmap_delete(page_data);

  return html;
}


struct response* get_new_post(const char* community_id, const struct auth_token* client_info)
{
  if (community_id == NULL)
  {
    return senderr(ERR_NOT_FOUND);
  }

  if (client_info == NULL)
  {
    return redirect("/login");
  }

  list* community_info;
  if ((community_info = 
}
