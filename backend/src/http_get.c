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
#include <file_manager.h>
#include <response.h>
#include <get_form.h>
#include <get_user.h>
#include <get_post.h>
#include <get_community.h>
#include <get_feed.h>
#include <http_get.h>

static void terminate_http_get();
static struct response* get_file(const char* uri);

static ks_hashmap* endpoints = NULL;


void init_http_get()
{
  endpoints = ks_hashmap_new(KS_CHARP, 32);

  // from get_feed.c
  add_map_value_vp(endpoints, "./", &get_home);
  add_map_value_vp(endpoints, "./home", &get_home);
  add_map_value_vp(endpoints, "./all", &get_all);
  add_map_value_vp(endpoints, "./communities", &get_communities);
  add_map_value_vp(endpoints, "./subscriptions", &get_subscriptions);

  // from get_user.c 
  add_map_value_vp(endpoints, "./user", &get_user);

  // from get_post.c
  add_map_value_vp(endpoints, "./post", &get_post);

  // from get_community.c
  add_map_value_vp(endpoints, "./community", &get_community);

  // from get_form.c
  add_map_value_vp(endpoints, "./login", &get_login);
  add_map_value_vp(endpoints, "./edit_user", &get_edit_user);
  add_map_value_vp(endpoints, "./new_post", &get_new_post);
  add_map_value_vp(endpoints, "./edit_post", &get_edit_post);
  add_map_value_vp(endpoints, "./new_comment", &get_new_comment);
  add_map_value_vp(endpoints, "./edit_comment", &get_edit_comment);
  add_map_value_vp(endpoints, "./new_community", &get_new_community);
  add_map_value_vp(endpoints, "./edit_community", &get_edit_community);

  atexit(&terminate_http_get);
}


struct response* http_get(const struct request* req)
{
  struct response* (*func) (const struct request*);
  if ((func = get_map_value_vp(endpoints, req->uri)) != NULL)
  {
    return func(req);
  }

  return get_file(req->uri);
}


static void terminate_http_get()
{
  ks_hashmap_delete(endpoints);
}


static struct response* get_file(const char* uri)
{
  // no '..' allowed
  if (strstr(uri, "..") != NULL)
  {
    return response_error(STAT404);
  }

  int uri_len = strlen(uri);
  char* content_type;

  // determine file type
  if (strstr(uri + (uri_len - 4), ".css") != NULL)
  {
    content_type = "Content-Type: text/css\r\n";
  }
  else if (strstr(uri + (uri_len - 3), ".js") != NULL)
  {
    content_type = "Content-Type: application/javascript\r\n";
  }
  else if (strstr(uri + (uri_len - 4), ".ico") != NULL)
  {
    content_type = "Content-Type: image/x-icon\r\n";
  }
  else
  {
    return response_error(STAT404);
  }

  // get file from file manager
  struct file_pkg* pkg;
  if ((pkg = load_file(uri)) == NULL)
  {
    return response_error(STAT404);
  }
  struct response* resp = calloc(1, sizeof(struct response));

  // expiration
  char* expiry = "Cache-Control: public, max-age=86400\r\n";

  // build response object
  char contlenline[80];
  int contlen = sprintf(contlenline, "Content-Length: %d\r\n", pkg->length);
  resp->header = ks_list_new();
  ks_list_add(resp->header, ks_datacont_new(STAT200, KS_CHARP, strlen(STAT200)));
  ks_list_add(resp->header, ks_datacont_new(expiry, KS_CHARP, strlen(expiry)));
  ks_list_add(resp->header, ks_datacont_new(content_type, KS_CHARP, strlen(content_type)));
  ks_list_add(resp->header, ks_datacont_new(contlenline, KS_CHARP, contlen));

  resp->content = pkg->contents;
  resp->content_length = pkg->length;

  free(pkg);

  return resp;
}


enum vote_type check_for_vote(enum item_type item_type, const char* item_id, const char* user_id)
{
  ks_hashmap* up_vote = NULL;
  ks_hashmap* down_vote = NULL;

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
  const char* date_str = NULL;

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


ks_hashmap* wrap_page_data(const struct auth_token* client_info, const ks_hashmap* page_data)
{
  // page_data cannot be NULL
  if (page_data == NULL)
  {
    log_crit("wrap_page_data(): NULL page_data");
  }

  // create wrapper hashmap
  ks_hashmap* wrapper = ks_hashmap_new(KS_CHARP, 8);

  // add main html path and page data
  add_map_value_str(wrapper, TEMPLATE_PATH_KEY, HTML_MAIN);
  add_map_value_hm(wrapper, PAGE_CONTENT_KEY, page_data);

  // add nav bar info
  char user_link[64];

  if (client_info != NULL)
  {
    sprintf(user_link, "/user?id=%s", client_info->user_id);
    add_map_value_str(wrapper, CLIENT_LINK_KEY, user_link);
    add_map_value_str(wrapper, CLIENT_NAME_KEY, client_info->user_name);
  }

  else
  {
    add_map_value_str(wrapper, CLIENT_LINK_KEY, "/login");
    add_map_value_str(wrapper, CLIENT_NAME_KEY, "Login");
  }

  return wrapper;
}
