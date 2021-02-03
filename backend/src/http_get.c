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
#include <get_feed.h>
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
  else if ((strcmp(req->uri, "./") == 0) ||
           (strcmp(req->uri, "./home") == 0))
  {
    content_type = TEXTHTML;
    if ((content = get_feed(HOME_FEED, req->client_info)) != NULL)
    {
      content_length = strlen(content);
    }
  }
  else if (strcmp(req->uri, "./popular") == 0)
  {
    content_type = TEXTHTML;
    if ((content = get_feed(POPULAR_FEED, req->client_info)) != NULL)
    {
      content_length = strlen(content);
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


enum vote_type check_for_vote(enum item_type item_type, const char* item_id, const char* user_id)
{
  ks_list* upvote_query_result;
  ks_list* downvote_query_result;

  // are we looking for post votes or comment votes?
  if (item_type == POST_ITEM)
  {
    upvote_query_result = query_post_upvotes_by_post_id_user_id(item_id, user_id);
    downvote_query_result = query_post_downvotes_by_post_id_user_id(item_id, user_id);
  }
  else if (item_type == COMMENT_ITEM)
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
    log_crit("check_for_vote(): downvote and upvote present for same %s_id=%s user_id=%s", item_type==POST_ITEM?"post":"comment", item_id, user_id);
  }
  if (uv0 != NULL && uv1 != NULL)
  {
    log_crit("check_for_vote(): multiple upvotes present for same %s_id=%s user_id=%s", item_type==POST_ITEM?"post":"comment", item_id, user_id);
  }
  if (dv0 != NULL && dv1 != NULL)
  {
    log_crit("check_for_vote(): multiple downvotes present for same %s_id=%s user_id=%s", item_type==POST_ITEM?"post":"comment", item_id, user_id);
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


time_t get_item_date(const ks_hashmap* item, enum item_type it)
{
  const char* date_str;

  if (it == POST_ITEM)
  {
    date_str = get_map_value(item, FIELD_POST_DATE_POSTED)->cp;
  }
  else if (it == COMMENT_ITEM)
  {
    date_str = get_map_value(item, FIELD_COMMENT_DATE_POSTED)->cp;
  }
  else log_crit("get_item_date(): Invalid item_type param");

  return (time_t) strtol(date_str, NULL, 10);
}


ks_list* sort_items(ks_list* items, enum item_type it)
{
  if (items == NULL)
  {
    return NULL;
  }

  int count = ks_list_length(items);

  // reverse item list
  // (odds are the items at the beginning of
  // list are older)
  ks_list* rev_list = ks_list_new();
  const ks_datacont* curr;
  ks_iterator* iter = ks_iterator_new(items, KS_LIST);
  while ((curr = ks_iterator_get(iter)) != NULL)
  {
    ks_list_insert(rev_list, ks_datacont_copy(curr), 0);
  }
  ks_iterator_delete(iter);
  ks_list_delete(items);

  // bubble sort, too lazy to do better right now
  // TODO: change this to quicksort or something
  items = rev_list;
  for (int i = 0; i < count; i++)
  {
    bool done = true;
    for (int j = 0; j < count-1-i; j++)
    {
      ks_datacont* itemA = ks_list_get(items, j);
      time_t tA = get_item_date(itemA->hm, it);

      ks_datacont* itemB = ks_list_get(items, j+1);
      time_t tB = get_item_date(itemB->hm, it);

      if (tA < tB)
      {
        done = false;

        // swap positions
        ks_datacont* temp = ks_datacont_copy(itemA);
        ks_list_remove_at(items, j);
        ks_list_insert(items, temp, j + 1);
      }
    }
    if (done == true)
    {
      break;
    }
  }

  return items;
}


ks_list* merge_items(ks_list* lsA, ks_list* lsB, enum item_type it)
{
  if (lsA == NULL || lsB == NULL)
  {
    return (lsA ? lsA : (lsB ? lsB : NULL));
  }

  ks_list* merged = ks_list_new();
  ks_iterator* iterA = ks_iterator_new(lsA, KS_LIST);
  ks_iterator* iterB = ks_iterator_new(lsB, KS_LIST);
  ks_datacont* itemA, *itemB;

  do
  {
    itemA = ks_datacont_copy(ks_iterator_get(iterA));
    time_t tA = itemA ? get_item_date(itemA->hm, it) : 0;

    itemB = ks_datacont_copy(ks_iterator_get(iterB));
    time_t tB = itemB ? get_item_date(itemB->hm, it) : 0;

    ks_datacont* first = tA < tB ? itemB : itemA;
    ks_datacont* second = tA < tB ? itemA : itemB;

    if (first != NULL)
    {
      ks_list_add(merged, first);
    }

    if (second != NULL)
    {
      ks_list_add(merged, second);
    }
  } while (itemA != NULL || itemB != NULL);

  ks_iterator_delete(iterA);
  ks_iterator_delete(iterB);
  ks_list_delete(lsA);
  ks_list_delete(lsB);

  return merged;
}
