#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <log_manager.h>
#include <senderr.h>
#include <auth_manager.h>
#include <sql_manager.h>
#include <string_map.h>
#include <templating.h>
#include <http_get.h>
#include <get_feed.h>


static ks_list* wrap_posts(ks_list* posts, const struct auth_token* client_info)
{
  if (posts == NULL)
  {
    return NULL;
  }

  // list of vote wrappers
  ks_list* post_wrappers = ks_list_new();

  // iterate through each post
  ks_datacont* post_info;
  ks_iterator* iter = ks_iterator_new(posts, KS_LIST);
  while ((post_info = (ks_datacont*) ks_iterator_get(iter)) != NULL)
  {
    // add user post template path
    add_map_value_str(post_info->hm, TEMPLATE_PATH_KEY, HTML_FEED_POST);

    // move post points and ID into vote wrapper map
    const char* post_id = get_map_value(post_info->hm, FIELD_POST_ID)->cp;
    const char* points = get_map_value(post_info->hm, FIELD_POST_POINTS)->cp;

    ks_hashmap* wrapper = ks_hashmap_new(KS_CHARP, 8);
    add_map_value_str(wrapper, TEMPLATE_PATH_KEY, HTML_POST_VOTE_WRAPPER);
    add_map_value_str(wrapper, POST_ID_KEY, post_id);
    add_map_value_str(wrapper, POST_POINTS_KEY, points);

    // check if client user voted on this post
    char* upvote_class = UPVOTE_NOTCLICKED_STATE;
    char* downvote_class = DOWNVOTE_NOTCLICKED_STATE;
    if (client_info != NULL)
    {
      enum vote_type vt = check_for_vote(POST_ITEM, post_id, client_info->user_id);
      upvote_class = vt == UPVOTE ? UPVOTE_CLICKED_STATE : upvote_class;
      downvote_class = vt == DOWNVOTE ? DOWNVOTE_CLICKED_STATE : downvote_class;
    }
    add_map_value_str(wrapper, UPVOTE_CLICKED_KEY, upvote_class);
    add_map_value_str(wrapper, DOWNVOTE_CLICKED_KEY, downvote_class);

    // add post info map to vote wrapper map
    add_map_value_hm(wrapper, POST_KEY, post_info->hm);
    post_info->hm = NULL;
    ks_list_add(post_wrappers, ks_datacont_new(wrapper, KS_HASHMAP, 1));
  }
  ks_iterator_delete(iter);
  ks_list_delete(posts);

  return post_wrappers;
}


struct response* get_feed(const enum feed_type ft, const struct auth_token* client_info)
{
  if (ft == HOME_FEED && client_info == NULL)
  {
    return redirect("/popular");
  }

  ks_hashmap* feed_info = ks_hashmap_new(KS_CHARP, 8);
  add_map_value_str(feed_info, TEMPLATE_PATH_KEY, HTML_FEED);

  // get list of posts
  ks_list* posts = NULL;
  if (ft == HOME_FEED)
  {
    add_map_value_str(feed_info, FEED_TITLE_KEY, "Home");

    // get list of subscriptions
    ks_list* subs;
    if ((subs = query_subscriptions_by_user_id(client_info->user_id)) != NULL)
    {
      posts = ks_list_new();

      // for each sub, get list of posts
      const ks_datacont* curr;
      ks_iterator* iter = ks_iterator_new(subs, KS_LIST);
      while ((curr = ks_iterator_get(iter)) != NULL)
      {
        const char* community_name = get_map_value(curr->hm, FIELD_SUB_COMMUNITY_NAME)->cp;
        ks_list* temp = query_posts_by_community_name(community_name);
        temp = sort_items(temp, POST_ITEM);
        posts = merge_items(posts, temp, POST_ITEM);
      }
      ks_iterator_delete(iter);
      ks_list_delete(subs);
    }
  }
  else
  {
    add_map_value_str(feed_info, FEED_TITLE_KEY, "All");

    posts = query_all_posts();

    posts = sort_items(posts, POST_ITEM);
  }
  posts = wrap_posts(posts, client_info);

  // add feed list to feed_info map
  if (posts == NULL)
  {
    add_map_value_str(feed_info, FEED_ITEM_LIST_KEY, "");
  }
  else
  {
    add_map_value_ls(feed_info, FEED_ITEM_LIST_KEY, posts);
  }

  // put page data together
  ks_hashmap* page_data = ks_hashmap_new(KS_CHARP, 8);
  add_map_value_hm(page_data, PAGE_CONTENT_KEY, feed_info);
  add_map_value_str(page_data, STYLE_PATH_KEY, CSS_FEED);
  add_map_value_str(page_data, SCRIPT_PATH_KEY, "");
  add_map_value_str(page_data, TEMPLATE_PATH_KEY, HTML_MAIN);
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


struct response* get_communities(const struct auth_token* client_info)
{
  // query communities and sort
  ks_list* communities = query_all_communities();
  communities = sort_items(communities, COMMUNITY_ITEM);

  // add html template path to each community
  const ks_datacont* curr;
  ks_iterator* iter = ks_iterator_new(communities, KS_LIST);
  while ((curr = ks_iterator_get(iter)) != NULL)
  {
    add_map_value_str(curr->hm, TEMPLATE_PATH_KEY, HTML_FEED_COMMUNITY);
  }
  ks_iterator_delete(iter);

  // build feed data
  ks_hashmap* feed_info = ks_hashmap_new(KS_CHARP, 4);
  add_map_value_str(feed_info, TEMPLATE_PATH_KEY, HTML_FEED);
  add_map_value_str(feed_info, FEED_TITLE_KEY, "Communities");
  add_map_value_ls(feed_info, FEED_ITEM_LIST_KEY, communities);

  // put page data together
  ks_hashmap* page_data = ks_hashmap_new(KS_CHARP, 8);
  add_map_value_hm(page_data, PAGE_CONTENT_KEY, feed_info);
  add_map_value_str(page_data, STYLE_PATH_KEY, CSS_FEED);
  add_map_value_str(page_data, SCRIPT_PATH_KEY, "");
  add_map_value_str(page_data, TEMPLATE_PATH_KEY, HTML_MAIN);
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
