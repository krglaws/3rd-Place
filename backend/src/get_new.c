#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <kylestructs.h>

#include <string_map.h>
#include <senderr.h>
#include <common.h>
#include <log_manager.h>
#include <auth_manager.h>
#include <templating.h>
#include <http_get.h>
#include <sql_manager.h>
#include <get_new.h>


struct response* get_login(const struct auth_token* client_info, enum login_error err)
{
  ks_hashmap* page_content = ks_hashmap_new(KS_CHARP, 8);

  // only get actual login html if user is not logged in yet
  if (client_info == NULL)
  {
    add_map_value_str(page_content, TEMPLATE_PATH_KEY, HTML_LOGIN);

    // check for login/signup error
    if (err == LOGINERR_BAD_LOGIN)
    {
      add_map_value_str(page_content, SIGNUP_ERROR_KEY, "");
      add_map_value_str(page_content, LOGIN_ERROR_KEY, BAD_LOGIN_MSG);
    }
    else if (err == LOGINERR_UNAME_TAKEN)
    {
      add_map_value_str(page_content, SIGNUP_ERROR_KEY, UNAME_TAKEN_MSG);
      add_map_value_str(page_content, LOGIN_ERROR_KEY, "");
    }
    else if (err == LOGINERR_EMPTY)
    {
      add_map_value_str(page_content, SIGNUP_ERROR_KEY, EMPTY_INPUT_MSG);
      add_map_value_str(page_content, LOGIN_ERROR_KEY, "");
    }
    else if (err == LOGINERR_NONE)
    {
      add_map_value_str(page_content, SIGNUP_ERROR_KEY, "");
      add_map_value_str(page_content, LOGIN_ERROR_KEY, "");
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
  add_map_value_str(page_data, SCRIPT_PATH_KEY, "");
  add_map_value_hm(page_data, PAGE_CONTENT_KEY, page_content);
  add_nav_info(page_data, client_info);

  struct response* resp = calloc(1, sizeof(struct response));

  // build template
  if ((resp->content = build_template(page_data)) == NULL)
  {
    free(resp);
    ks_hashmap_delete(page_data);
    return senderr(ERR_INTERNAL);
  }
  ks_hashmap_delete(page_data);

  // prepare response object
  resp->content_length = strlen(resp->content);
  char contlenline[80];
  int contlen = sprintf(contlenline, "Content-Length: %d\r\n", resp->content_length);
  resp->header = ks_list_new();
  ks_list_add(resp->header, ks_datacont_new(STAT200, KS_CHARP, strlen(STAT200)));
  ks_list_add(resp->header, ks_datacont_new(TEXTHTML, KS_CHARP, strlen(TEXTHTML)));
  ks_list_add(resp->header, ks_datacont_new(contlenline, KS_CHARP, contlen));

  return resp;
}


struct response* get_new_post(const char* community_name, const struct auth_token* client_info)
{
  if (client_info == NULL)
  {
    return redirect("/login");
  }

  // get community info
  ks_hashmap* community_info;
  if (community_name == NULL || (community_info = get_community_info(community_name)) == NULL)
  {
    return senderr(ERR_NOT_FOUND);
  }
  add_map_value_str(community_info, TEMPLATE_PATH_KEY, HTML_NEW_POST);

  // put page data together
  ks_hashmap* page_data = ks_hashmap_new(KS_CHARP, 8);
  add_map_value_str(page_data, STYLE_PATH_KEY, CSS_FORM);
  add_map_value_str(page_data, SCRIPT_PATH_KEY, "");
  add_map_value_str(page_data, TEMPLATE_PATH_KEY, HTML_MAIN);
  add_map_value_hm(page_data, PAGE_CONTENT_KEY, community_info);
  add_nav_info(page_data, client_info);

  struct response* resp = calloc(1, sizeof(struct response));

  // build template
  if ((resp->content = build_template(page_data)) == NULL)
  {
    free(resp);
    ks_hashmap_delete(page_data);
    return senderr(ERR_INTERNAL);
  }
  ks_hashmap_delete(page_data);

  // prepare response object
  resp->content_length = strlen(resp->content);
  char contlenline[80];
  int contlen = sprintf(contlenline, "Content-Length: %d\r\n", resp->content_length);
  resp->header = ks_list_new();
  ks_list_add(resp->header, ks_datacont_new(STAT200, KS_CHARP, strlen(STAT200)));
  ks_list_add(resp->header, ks_datacont_new(TEXTHTML, KS_CHARP, strlen(TEXTHTML)));
  ks_list_add(resp->header, ks_datacont_new(contlenline, KS_CHARP, contlen));

  return resp;
}


struct response* get_new_comment(const char* post_id, const struct auth_token* client_info)
{
  if (client_info == NULL)
  {
    return redirect("/login");
  }

  // get post info
  ks_hashmap* post_info;
  if (post_id == NULL || (post_info = get_post_info(post_id)) == NULL)
  {
    return senderr(ERR_NOT_FOUND);
  }
  add_map_value_str(post_info, TEMPLATE_PATH_KEY, HTML_NEW_COMMENT);

  // put page data together
  ks_hashmap* page_data = ks_hashmap_new(KS_CHARP, 8);
  add_map_value_str(page_data, STYLE_PATH_KEY, CSS_FORM);
  add_map_value_str(page_data, SCRIPT_PATH_KEY, "");
  add_map_value_str(page_data, TEMPLATE_PATH_KEY, HTML_MAIN);
  add_map_value_hm(page_data, PAGE_CONTENT_KEY, post_info);
  add_nav_info(page_data, client_info);

  struct response* resp = calloc(1, sizeof(struct response));

  // build template
  if ((resp->content = build_template(page_data)) == NULL)
  {
    free(resp);
    ks_hashmap_delete(page_data);
    return senderr(ERR_INTERNAL);
  }
  ks_hashmap_delete(page_data);

  // prepare response object
  resp->content_length = strlen(resp->content);
  char contlenline[80];
  int contlen = sprintf(contlenline, "Content-Length: %d\r\n", resp->content_length);
  resp->header = ks_list_new();
  ks_list_add(resp->header, ks_datacont_new(STAT200, KS_CHARP, strlen(STAT200)));
  ks_list_add(resp->header, ks_datacont_new(TEXTHTML, KS_CHARP, strlen(TEXTHTML)));
  ks_list_add(resp->header, ks_datacont_new(contlenline, KS_CHARP, contlen));

  return resp;
}


struct response* get_new_community(const struct auth_token* client_info)
{
  if (client_info == NULL)
  {
    return redirect("/login");
  }

  ks_hashmap* form_data = ks_hashmap_new(KS_CHARP, 8);
  add_map_value_str(form_data, TEMPLATE_PATH_KEY, HTML_NEW_COMMUNITY);

  // put page data together
  ks_hashmap* page_data = ks_hashmap_new(KS_CHARP, 8);
  add_map_value_str(page_data, STYLE_PATH_KEY, CSS_FORM);
  add_map_value_str(page_data, SCRIPT_PATH_KEY, "");
  add_map_value_str(page_data, TEMPLATE_PATH_KEY, HTML_MAIN);
  add_map_value_hm(page_data, PAGE_CONTENT_KEY, form_data);
  add_nav_info(page_data, client_info);

  struct response* resp = calloc(1, sizeof(struct response));

  // build template
  if ((resp->content = build_template(page_data)) == NULL)
  {
    free(resp);
    ks_hashmap_delete(page_data);
    return senderr(ERR_INTERNAL);
  }
  ks_hashmap_delete(page_data);

  // prepare response object
  resp->content_length = strlen(resp->content);
  char contlenline[80];
  int contlen = sprintf(contlenline, "Content-Length: %d\r\n", resp->content_length);
  resp->header = ks_list_new();
  ks_list_add(resp->header, ks_datacont_new(STAT200, KS_CHARP, strlen(STAT200)));
  ks_list_add(resp->header, ks_datacont_new(TEXTHTML, KS_CHARP, strlen(TEXTHTML)));
  ks_list_add(resp->header, ks_datacont_new(contlenline, KS_CHARP, contlen));

  return resp;
}
