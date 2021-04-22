#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <kylestructs.h>

#include <templating.h>
#include <response.h>
#include <string_map.h>
#include <log_manager.h>
#include <sql_manager.h>
#include <http_get.h>
#include <get_community.h>


static ks_list* get_community_posts(const char* community_id, const struct auth_token* client_info)
{
  // get community posts
  ks_list* posts;
  if ((posts = query_posts_by_community_id(community_id)) == NULL)
  {
    return NULL;
  }
  posts = sort_list(posts, POST_ITEM);

  // vote wrapper list
  ks_list* post_wrappers = ks_list_new();

  // iterate through each post
  ks_datacont* post_info;
  ks_iterator* iter = ks_iterator_new(posts, KS_LIST);
  while ((post_info = (ks_datacont*) ks_iterator_next(iter)) != NULL)
  {
    // add user post template path
    add_map_value_str(post_info->hm, TEMPLATE_PATH_KEY, HTML_COMMUNITY_POST);

    // move post points and ID into vote wrapper map
    const char* post_id = get_map_value_str(post_info->hm, FIELD_POST_ID);
    const char* points = get_map_value_str(post_info->hm, FIELD_POST_POINTS);

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


static ks_list* get_community_moderators(const char* community_id, bool is_owner)
{
  ks_list* mod_list;
  if ((mod_list = query_moderators_by_community_id(community_id)) == NULL)
  {
    return NULL;
  }

  char* vis_key = is_owner ? "visible" : "hidden";

  const ks_datacont* curr;
  ks_iterator* iter = ks_iterator_new(mod_list, KS_LIST);
  while ((curr = ks_iterator_next(iter)) != NULL)
  {
    add_map_value_str(curr->hm, TEMPLATE_PATH_KEY, HTML_COMMUNITY_MOD_ENTRY);
    add_map_value_str(curr->hm, DELETE_OPTION_VISIBILITY_KEY, vis_key);
  }
  ks_iterator_delete(iter);

  return mod_list;
}


struct response* get_community(const struct request* req)
{
  const char* community_id = get_map_value_str(req->query, "id");
  const char* community_name = get_map_value_str(req->query, "name");

  // get user info from DB
  ks_hashmap* page_data;
  if (community_id != NULL)
  {
    if ((page_data = query_community_by_id(community_id)) == NULL)
    {
      return response_error(STAT404);
    }
    community_name = get_map_value_str(page_data, FIELD_COMMUNITY_NAME);
  }
  else if (community_name != NULL)
  {
    if ((page_data = query_community_by_name(community_name)) == NULL)
    {
      return response_error(STAT404);
    }
    community_id = get_map_value_str(page_data, FIELD_COMMUNITY_ID);
  }
  else
  {
    return response_error(STAT404);
  }

  add_map_value_str(page_data, TEMPLATE_PATH_KEY, HTML_COMMUNITY);

  // check if client is subscribed
  char* substatus = "subscribe";
  ks_hashmap* sub_info;
  if (req->client_info != NULL &&
      (sub_info = query_subscription_by_community_id_user_id(community_id, req->client_info->user_id)) != NULL)
  {
    ks_hashmap_delete(sub_info);
    substatus = "unsubscribe";
  }
  add_map_value_str(page_data, COMMUNITY_SUBSCRIPTION_STATUS_KEY, substatus);

  // get community posts
  ks_list* community_posts;
  if ((community_posts = get_community_posts(community_id, req->client_info)) != NULL)
  {
    add_map_value_ls(page_data, COMMUNITY_POST_LIST_KEY, community_posts);
  }
  else
  {
    add_map_value_str(page_data, COMMUNITY_POST_LIST_KEY, "<p><em>No posts yet...</em></p>");
  }

  bool is_owner = false;
  char* edit_vis = "hidden"; 
  char* delete_vis = "hidden";
  if (req->client_info != NULL)
  {
    const char* owner_id = get_map_value_str(page_data, FIELD_COMMUNITY_OWNER_ID);

    ks_hashmap* admin_info;

    // check if client is owner
    if (strcmp(owner_id, req->client_info->user_id) == 0)
    {
      edit_vis = "visible";
      delete_vis = "visible";
      is_owner = true;
    }
    // check if client is admin
    else if ((admin_info = query_administrator_by_user_id(req->client_info->user_id)) != NULL)
    {
      ks_hashmap_delete(admin_info);
      delete_vis = "visible";
    }
  }
  add_map_value_str(page_data, EDIT_OPTION_VISIBILITY_KEY, edit_vis);
  add_map_value_str(page_data, DELETE_OPTION_VISIBILITY_KEY, delete_vis);

  // get community moderators
  ks_list* community_mods;
  if ((community_mods = get_community_moderators(community_id, is_owner)) != NULL)
  {
    add_map_value_ls(page_data, COMMUNITY_MOD_LIST_KEY, community_mods);
  }
  else
  {
    add_map_value_str(page_data, COMMUNITY_MOD_LIST_KEY, "No moderators yet<br>");
  }

  // put page data together
  page_data = wrap_page_data(req->client_info, page_data);

  // build content
  char* content;
  if ((content = build_template(page_data)) == NULL)
  {
    ks_hashmap_delete(page_data);
    return response_error(STAT500);
  }
  ks_hashmap_delete(page_data);

  return response_ok(content);
}
