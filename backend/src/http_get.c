#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <kylestructs.h>

#include <log_manager.h>
#include <auth_manager.h>
#include <sql_manager.h>
#include <templating.h>
#include <string_map.h>
#include <server.h>
#include <load_file.h>
#include <response.h>
#include <get_form.h>
#include <get_user.h>
#include <get_post.h>
#include <get_community.h>
#include <get_feed.h>
#include <http_get.h>


struct response* http_get(const struct request* req)
{
  if (req == NULL)
  {
    log_crit("http_get(): NULL request object");
  }

  if (strcmp(req->uri, "./") == 0 ||
      strcmp(req->uri, "./home") == 0)
  {
    return get_feed(HOME_FEED, req->client_info);
  }

  if (strcmp(req->uri, "./all") == 0)
  {
    return get_feed(POPULAR_FEED, req->client_info);
  }

  if (strcmp(req->uri, "./communities") == 0)
  {
    return get_communities(req->client_info);
  }

  if (strcmp(req->uri, "./login") == 0)
  {
    return get_login(NULL, USER_FORM_ERR_NONE, req->client_info);
  }

  if (strcmp(req->uri, "./edit_user") == 0)
  {
    return get_edit_user(NULL, USER_FORM_ERR_NONE, req->client_info);
  }

  if (strcmp(req->uri, "./new_post") == 0)
  {
    const char* community_id = get_map_value_str(req->query, "community_id");
    return get_new_post(NULL, NULL, community_id, POST_FORM_ERR_NONE, req->client_info);
  }

  if (strcmp(req->uri, "./edit_post") == 0)
  {
    const char* post_id = get_map_value_str(req->query, "post_id");
    return get_edit_post(NULL, post_id, POST_FORM_ERR_NONE, req->client_info);
  }

  if (strcmp(req->uri, "./new_comment") == 0)
  {
    const char* post_id = get_map_value_str(req->query, "post_id");
    return get_new_comment(NULL, post_id, COMMENT_FORM_ERR_NONE, req->client_info);
  }

  if (strcmp(req->uri, "./edit_comment") == 0)
  {
    const char* comment_id = get_map_value_str(req->query, "comment_id");
    return get_edit_comment(NULL, comment_id, COMMENT_FORM_ERR_NONE, req->client_info);
  }

  if (strcmp(req->uri, "./new_community") == 0)
  {
    return get_new_community(NULL, NULL, COMMUNITY_FORM_ERR_NONE, req->client_info);
  }

  if (strcmp(req->uri, "./edit_community") == 0)
  {
    const char* community_id = get_map_value_str(req->query, "community_id");
    return get_edit_community(NULL, community_id, COMMUNITY_FORM_ERR_NONE, req->client_info);
  }

  if (req->uri == strstr(req->uri, "./u/"))
  {
    return get_user(req->uri+4, req->client_info);
  }

  if (req->uri == strstr(req->uri, "./p/"))
  {
    return get_post(req->uri+4, req->client_info);
  }

  if (req->uri == strstr(req->uri, "./c/"))
  {
    return get_community(req->uri+4, req->client_info);
  }

  return get_file(req->uri);
}


struct response* get_file(const char* uri)
{
  // no '..' allowed
  if (strstr(uri, "..") != NULL)
  {
    return response_error(STAT404);
  }

  // get file
  struct file_str* f;
  if (access(uri, F_OK) != 0 || (f = load_file_str(uri)) == NULL)
  {
    return response_error(STAT404);
  }

  struct response* resp = calloc(1, sizeof(struct response));
  char* content_type;

  // determine file type
  if (strstr(uri, ".css") != NULL)
  {
    content_type = TEXTCSS;
  }
  else if (strstr(uri, ".js") != NULL)
  {
    content_type = APPJS;
  }
  else if (strstr(uri, ".ico") != NULL)
  {
    content_type = IMGICO;
  }

  // build response object
  char contlenline[80];
  int contlen = sprintf(contlenline, "Content-Length: %d\n", f->len);
  resp->header = ks_list_new();
  ks_list_add(resp->header, ks_datacont_new(STAT200, KS_CHARP, strlen(STAT200)));
  ks_list_add(resp->header, ks_datacont_new(content_type, KS_CHARP, strlen(content_type)));
  ks_list_add(resp->header, ks_datacont_new(contlenline, KS_CHARP, contlen));

  resp->content = f->contents;
  resp->content_length = f->len;

  free(f);

  return resp;
}


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
  ks_hashmap* up_vote;
  ks_hashmap* down_vote;

  // are we looking for post votes or comment votes?
  if (item_type == POST_ITEM)
  {
    up_vote = query_post_up_vote_by_post_id_user_id(item_id, user_id);
    down_vote = query_post_down_vote_by_post_id_user_id(item_id, user_id);
  }
  else if (item_type == COMMENT_ITEM)
  {
    up_vote = query_comment_up_vote_by_comment_id_user_id(item_id, user_id);
    down_vote = query_comment_down_vote_by_comment_id_user_id(item_id, user_id);
  }
  else {
    log_crit("check_for_vote(): invalid item_type argument");
  }

  // determine vote type
  enum vote_type result = up_vote ? UPVOTE : (down_vote ? DOWNVOTE : NOVOTE);

  // cleanup query results
  ks_hashmap_delete(up_vote);
  ks_hashmap_delete(down_vote);

  return result;
} // end check_for_vote()


bool check_for_sub(const char* community_id, const struct auth_token* client_info)
{
  if (community_id == NULL)
  {
    log_err("check_for_sub(): NULL community_id parameter");
    return false;
  }

  if (client_info == NULL)
  {
    log_err("check_for_sub(): NULL client_info parameter");
    return false;
  }

  ks_hashmap* sub = query_subscription_by_community_id_user_id(community_id, client_info->user_id);
  bool subbed = sub != NULL;
  ks_hashmap_delete(sub);

  return subbed;
}


time_t get_item_date(const ks_hashmap* item, enum item_type it)
{
  const char* date_str;

  if (it == POST_ITEM)
  {
    date_str = get_map_value_str(item, FIELD_POST_DATE_POSTED);
  }
  else if (it == COMMENT_ITEM)
  {
    date_str = get_map_value_str(item, FIELD_COMMENT_DATE_POSTED);
  }
  else if (it == COMMUNITY_ITEM)
  {
    date_str = get_map_value_str(item, FIELD_COMMUNITY_DATE_CREATED);
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


ks_hashmap* wrap_page_data(const struct auth_token* client_info, const ks_hashmap* page_data, const char* css_path, const char* js_path)
{
  // page_data cannot be NULL
  if (page_data == NULL)
  {
    log_crit("wrap_page_data(): NULL page_data");
  }

  // create wrapper hashmap
  ks_hashmap* wrapper = ks_hashmap_new(KS_CHARP, 8);

  // add paths and page data
  add_map_value_str(wrapper, TEMPLATE_PATH_KEY, HTML_MAIN);

  if (css_path != NULL)
  {
    add_map_value_str(wrapper, STYLE_PATH_KEY, css_path);
  }

  if (js_path != NULL)
  {
    add_map_value_str(wrapper, SCRIPT_PATH_KEY, js_path);
  }

  add_map_value_hm(wrapper, PAGE_CONTENT_KEY, page_data);

  // add nav bar info
  add_nav_info(wrapper, client_info);

  return wrapper;
}
