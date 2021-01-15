#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <kylestructs.h>

#include <log_manager.h>
#include <auth_manager.h>
#include <sql_manager.h>
#include <templating.h>
#include <string_map.h>
#include <common.h>
#include <load_file.h>
#include <senderr.h>
#include <get_user.h>
#include <get_post.h>
#include <get_community.h>
#include <http_get.h>


// used to signal errors while getting 
static enum get_err gerr = 0;


struct response* http_get(struct request* req)
{
  if (req == NULL) {
    return NULL;
  }

  // what are we getting?
  char* content_type = NULL;
  char* content = NULL;
  int content_length = 0;
  if (strstr(req->uri, ".css"))
  {
    content_type = TEXTCSS;
    if (content = load_file(req->uri))
    {
      content_length = strlen(content);
    }
  }
  else if (strstr(req->uri, ".js"))
  {
    content_type = APPJS;
    if (content = load_file(req->uri))
    {
      content_length = strlen(content);
    }
  }
  else if (strstr(req->uri, ".ico"))
  {
    struct file_str* ico;
    content_type = IMGICO;
    if (ico = load_file_str(req->uri))
    {
      content = ico->contents;
      content_length = ico->len;
      free(ico);
    }
  }
  else if (strstr(req->uri, "./u/"))
  {
    content_type = TEXTHTML;
    if (content = get_user(req->uri + 4, req->client_info))
    {
      content_length = strlen(content);
    }
  }
  else if (strstr(req->uri, "./c/"))
  {
    content_type = TEXTHTML;
    if (content = get_community(req->uri + 4, req->client_info))
    {
      content_length = strlen(content);
    }
  }
  else if (strstr(req->uri, "./p/"))
  {
    content_type = TEXTHTML;
    if (content = get_post(req->uri + 4, req->client_info))
    {
      content_length = strlen(content);
    }
  }
  else if (strcmp(req->uri, "./login") == 0)
  {
    content_type = TEXTHTML;
    if (content = get_login(req->client_info, LOGINERR_NONE))
    {
      content_length = strlen(content);
    }
  }

  // do we have it?
  if (content_type == NULL || content == NULL)
  {
    if (content != NULL)
    {
      free(content);
    }

    // get error type
    enum get_err err = get_error();

    // redirect
    if (err == REDIRECT)
    {
      return redirect("/login");
    }

    // check for 500 error
    if (get_error() == 1)
    {
      return senderr(ERR_INTERNAL);
    }

    // default to 404
    return senderr(ERR_NOT_FOUND);
  }

  // prepare response object
  struct response* resp = calloc(1, sizeof(struct response));
  resp->header = ks_list_new();
  ks_list_add(resp->header, ks_datacont_new(STAT200, KS_CHARP, strlen(STAT200)));
  ks_list_add(resp->header, ks_datacont_new(content_type, KS_CHARP, strlen(content_type)));
 
  char contlenline[80];
  int len = sprintf(contlenline, "Content-Length: %d\n", content_length);

  ks_list_add(resp->header, ks_datacont_new(contlenline, KS_CHARP, len));
  resp->content = content;
  resp->content_length = content_length;

  return resp;
} // end http_get()


void add_nav_info(ks_hashmap* page_data, const struct auth_token* client_info)
{
  char user_link[64];

  if (client_info != NULL)
  {
    sprintf(user_link, "/u/%s", client_info->user_name);
    add_map_value_str(page_data, CLIENT_LINK_KEY, user_link);
    add_map_value_str(page_data, CLIENT_NAME_KEY, client_info->user_name);
  }

  else
  {
    add_map_value_str(page_data, CLIENT_LINK_KEY, "/login");
    add_map_value_str(page_data, CLIENT_NAME_KEY, "Login");
  }
}


enum vote_type check_for_vote(const enum vote_item_type item_type, const char* item_id, const char* user_id)
{
  ks_list* upvote_query_result;
  ks_list* downvote_query_result;

  // are we looking for post votes or comment votes?
  if (item_type == POST_VOTE)
  {
    upvote_query_result = query_post_upvotes_by_post_id_user_id(item_id, user_id);
    downvote_query_result = query_post_downvotes_by_post_id_user_id(item_id, user_id);
  }
  else if (item_type == COMMENT_VOTE)
  {
    upvote_query_result = query_comment_upvotes_by_post_id_user_id(item_id, user_id);
    downvote_query_result = query_comment_downvotes_by_post_id_user_id(item_id, user_id);
  }
  else {
    log_crit("check_for_vote(): invalid item_type argument");
  }

  ks_datacont* uv0 = ks_list_get(upvote_query_result, 0);
  ks_datacont* uv1 = ks_list_get(upvote_query_result, 1);

  ks_datacont* dv0 = ks_list_get(downvote_query_result, 0);
  ks_datacont* dv1 = ks_list_get(downvote_query_result, 1);

  // NOTE:
  // might remove these checks
  if (uv0 != NULL && dv0 != NULL)
  {
    log_crit("check_for_vote(): downvote and upvote present for same %s_id=%s user_id=%s", item_type==POST_VOTE?"post":"comment", item_id, user_id);
  }
  if (uv0 != NULL && uv1 != NULL)
  {
    log_crit("check_for_vote(): multiple upvotes present for same %s_id=%s user_id=%s", item_type==POST_VOTE?"post":"comment", item_id, user_id);
  }
  if (dv0 != NULL && dv1 != NULL)
  {
    log_crit("check_for_vote(): multiple downvotes present for same %s_id=%s user_id=%s", item_type==POST_VOTE?"post":"comment", item_id, user_id);
  }

  // determine vote type
  enum vote_type result = uv0 ? UPVOTE : (dv0 ? DOWNVOTE : NOVOTE);

  // cleanup query results
  ks_list_delete(upvote_query_result);
  ks_list_delete(downvote_query_result);

  return result;
} // end check_for_vote()


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


void set_error(enum get_err err)
{
  gerr = err;
}


static enum get_err get_error()
{
  enum get_err err = gerr;
  gerr = NO_GET_ERR;
  return err;
}
