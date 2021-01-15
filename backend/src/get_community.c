#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <kylestructs.h>

#include <templating.h>
#include <string_map.h>
#include <log_manager.h>
#include <load_file.h>
#include <sql_manager.h>
#include <http_get.h>
#include <get_community.h>


static ks_hashmap* get_community_info(const char* community_name)
{
  ks_list* result;
  if ((result = query_communities_by_name(community_name)) == NULL)
  {
    return NULL;
  }

  ks_datacont* row0 = ks_list_get(result, 0);

  if (row0 == NULL)
  {
    ks_list_delete(result);
    return NULL;
  }

  ks_hashmap* community_info = row0->hm;
  row0->hm = NULL;
  ks_list_delete(result);

  return community_info;
}


static ks_list* get_community_posts(const char* community_name, const struct auth_token* client_info)
{
  // get community posts
  ks_list* posts;
  if ((posts = query_posts_by_community_name(community_name)) == NULL)
  {
    return NULL;
  }

  // vote wrapper list
  ks_list* post_wrappers = ks_list_new();

  // iterate through each post
  ks_datacont* post_info;
  ks_iterator* iter = ks_iterator_new(posts, KS_LIST);
  while ((post_info = (ks_datacont*) ks_iterator_get(iter)) != NULL)
  {
    // add user post template path
    add_map_value_str(post_info->hm, TEMPLATE_PATH_KEY, HTML_COMMUNITY_POST);

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
      enum vote_type vt = check_for_vote(POST_VOTE, post_id, client_info->user_id);
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


char* get_community(const char* community_name, const struct auth_token* client_info)
{
  if (community_name == NULL || strlen(community_name) == 0)
  {
    return NULL;
  }

  // get community info
  ks_hashmap* community_info;
  if ((community_info = get_community_info(community_name)) == NULL)
  {
    // community does not exist
    return NULL;
  }

  add_map_value_str(community_info, TEMPLATE_PATH_KEY, HTML_COMMUNITY);

  // get community posts
  ks_list* community_posts;
  if ((community_posts = get_community_posts(community_name, client_info)) == NULL)
  {
    add_map_value_str(community_info, COMMUNITY_POST_LIST_KEY, " ");
  }
  else
  {
    add_map_value_ls(community_info, COMMUNITY_POST_LIST_KEY, community_posts);
  }

  // put page data together
  ks_hashmap* page_data = ks_hashmap_new(KS_CHARP, 8);
  add_map_value_hm(page_data, PAGE_CONTENT_KEY, community_info);
  add_map_value_str(page_data, STYLE_PATH_KEY, CSS_COMMUNITY);
  add_map_value_str(page_data, SCRIPT_PATH_KEY, JS_COMMUNITY);
  add_map_value_str(page_data, TEMPLATE_PATH_KEY, HTML_MAIN);
  add_nav_info(page_data, client_info);

  // build template
  char* html = build_template(page_data);
  ks_hashmap_delete(page_data);

  return html;
}
