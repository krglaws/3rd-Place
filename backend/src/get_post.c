#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <kylestructs.h>

#include <templating.h>
#include <response.h>
#include <string_map.h>
#include <log_manager.h>
#include <load_file.h>
#include <auth_manager.h>
#include <sql_manager.h>
#include <http_get.h>
#include <get_post.h>


static ks_list* get_post_comments(const char* post_id, bool can_delete, const struct auth_token* client_info)
{
  // get user comments
  ks_list* comments;
  if ((comments = query_comments_by_post_id(post_id)) == NULL)
  {
    return NULL;
  }
  comments = sort_items(comments, COMMENT_ITEM);

  // list of vote wrappers
  ks_list* comment_wrappers = ks_list_new();

  // iterate through each post
  ks_datacont* comment_info;
  ks_iterator* iter = ks_iterator_new(comments, KS_LIST);
  while ((comment_info = (ks_datacont*) ks_iterator_get(iter)) != NULL)
  {
    // add user comment template path
    add_map_value_str(comment_info->hm, TEMPLATE_PATH_KEY, HTML_POST_COMMENT);

    // move comment points and ID into vote wrapper map
    const char* comment_id = get_map_value_str(comment_info->hm, FIELD_COMMENT_ID);
    const char* points = get_map_value_str(comment_info->hm, FIELD_COMMENT_POINTS);

    ks_hashmap* wrapper = ks_hashmap_new(KS_CHARP, 8);
    add_map_value_str(wrapper, TEMPLATE_PATH_KEY, HTML_COMMENT_VOTE_WRAPPER);
    add_map_value_str(wrapper, COMMENT_ID_KEY, comment_id);
    add_map_value_str(wrapper, COMMENT_POINTS_KEY, points);

    // check if client user voted on this comment
    char* upvote_class = UPVOTE_NOTCLICKED_STATE;
    char* downvote_class = DOWNVOTE_NOTCLICKED_STATE;
    if (client_info != NULL)
    {
      enum vote_type vt = check_for_vote(COMMENT_ITEM, comment_id, client_info->user_id);
      upvote_class = vt == UPVOTE ? UPVOTE_CLICKED_STATE : upvote_class;
      downvote_class = vt == DOWNVOTE ? DOWNVOTE_CLICKED_STATE : downvote_class;
    }
    add_map_value_str(wrapper, UPVOTE_CLICKED_KEY, upvote_class);
    add_map_value_str(wrapper, DOWNVOTE_CLICKED_KEY, downvote_class);

    const char* author_name = get_map_value_str(comment_info->hm, FIELD_COMMENT_AUTHOR_NAME);

    // add css visibility values for post options
    char* edit_vis = "hidden";
    char* delete_vis = "hidden";
    if (client_info != NULL)
    {
      // check if client wrote this comment
      if (strcmp(author_name, client_info->user_name) == 0)
      {
        edit_vis = "visible";
        delete_vis = "visible";
      }
      // check if client is amdin, mod, or community owner
      else if (can_delete)
      {
        delete_vis = "visible";
      }
    }
    add_map_value_str(comment_info->hm, EDIT_OPTION_VISIBILITY_KEY, edit_vis);
    add_map_value_str(comment_info->hm, DELETE_OPTION_VISIBILITY_KEY, delete_vis);

    // add post info map to vote wrapper map
    add_map_value_hm(wrapper, COMMENT_KEY, comment_info->hm);
    comment_info->hm = NULL;
    ks_list_add(comment_wrappers, ks_datacont_new(wrapper, KS_HASHMAP, 1));
  }
  ks_iterator_delete(iter);
  ks_list_delete(comments);

  return comment_wrappers;
}


struct response* get_post(const char* post_id, const struct auth_token* client_info)
{
  if (post_id == NULL || strlen(post_id) == 0)
  {
    return response_error(STAT404);
  }

  // get post from DB
  ks_hashmap* page_data;
  if ((page_data = query_post_by_id(post_id)) == NULL)
  {
    return response_error(STAT404);
  }

  // check if client has voted on this post
  char* upvote_class = UPVOTE_NOTCLICKED_STATE;
  char* downvote_class = DOWNVOTE_NOTCLICKED_STATE;
  if (client_info != NULL)
  {
    enum vote_type vt = check_for_vote(POST_ITEM, post_id, client_info->user_id);
    upvote_class = vt == UPVOTE ? UPVOTE_CLICKED_STATE : upvote_class;
    downvote_class = vt == DOWNVOTE ? DOWNVOTE_CLICKED_STATE : downvote_class;
  }
  add_map_value_str(page_data, UPVOTE_CLICKED_KEY, upvote_class);
  add_map_value_str(page_data, DOWNVOTE_CLICKED_KEY, downvote_class);
  add_map_value_str(page_data, TEMPLATE_PATH_KEY, HTML_POST);

  // add css visibility values for post options
  bool can_delete = false;
  char* edit_vis = "hidden";
  char* delete_vis = "hidden";
  if (client_info != NULL)
  {
    const char* author_name = get_map_value_str(page_data, FIELD_POST_AUTHOR_NAME);
    const char* community_id = get_map_value_str(page_data, FIELD_POST_COMMUNITY_ID);
    ks_hashmap* community_info = query_community_by_id(community_id);
    const char* owner_id = get_map_value_str(community_info, FIELD_COMMUNITY_OWNER_ID);
    ks_hashmap* mod_info = query_moderator_by_community_id_user_id(community_id, client_info->user_id);
    ks_hashmap* admin_info = query_administrator_by_user_id(client_info->user_id);

    can_delete = (mod_info != NULL) || (admin_info != NULL) || (strcmp(owner_id, client_info->user_id) == 0);

    ks_hashmap_delete(mod_info);
    ks_hashmap_delete(admin_info);
    ks_hashmap_delete(community_info);

    if (strcmp(author_name, client_info->user_name) == 0)
    {
      edit_vis = "visible";
      delete_vis = "visible";
    }
    else if (can_delete)
    {
      delete_vis = "visible";
    }
  }
  add_map_value_str(page_data, EDIT_OPTION_VISIBILITY_KEY, edit_vis);
  add_map_value_str(page_data, DELETE_OPTION_VISIBILITY_KEY, delete_vis);

  // get post comments
  ks_list* comments;
  if ((comments = get_post_comments(post_id, can_delete, client_info)) != NULL)
  {
    add_map_value_ls(page_data, POST_COMMENT_LIST_KEY, comments);
  }

  // put page data together
  page_data = wrap_page_data(client_info, page_data);

  // build template
  char* content;
  if ((content = build_template(page_data)) == NULL)
  {
    ks_hashmap_delete(page_data);
    return response_error(STAT500);
  }
  ks_hashmap_delete(page_data);

  // prepare response object
  return response_ok(content);
}
