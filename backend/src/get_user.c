#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <kylestructs.h>

#include <string_map.h>
#include <response.h>
#include <log_manager.h>
#include <auth_manager.h>
#include <templating.h>
#include <http_get.h>
#include <sql_manager.h>
#include <get_user.h>


static ks_list* get_user_posts(const char* user_name, const struct auth_token* client_info)
{
  // get user posts
  ks_list* posts;
  if ((posts = query_posts_by_author_name(user_name)) == NULL)
  {
    return NULL;
  }
  posts = sort_items(posts, POST_ITEM);

  // list of vote wrappers
  ks_list* post_wrappers = ks_list_new();

  // iterate through each post
  ks_datacont* post_info;
  ks_iterator* iter = ks_iterator_new(posts, KS_LIST);
  while ((post_info = (ks_datacont*) ks_iterator_get(iter)) != NULL)
  {
    // add user post template path
    add_map_value_str(post_info->hm, TEMPLATE_PATH_KEY, HTML_USER_POST);

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


static ks_list* get_user_comments(const char* user_name, const struct auth_token* client_info)
{
  // get user comments
  ks_list* comments;
  if ((comments = query_comments_by_author_name(user_name)) == NULL)
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
    add_map_value_str(comment_info->hm, TEMPLATE_PATH_KEY, HTML_USER_COMMENT);

    // move comment points and ID into vote wrapper map
    const char* comment_id = get_map_value_str(comment_info->hm, FIELD_COMMENT_ID);
    const char* points = get_map_value_str(comment_info->hm, FIELD_COMMENT_POINTS);

    ks_hashmap* wrapper = ks_hashmap_new(KS_CHARP, 8);
    add_map_value_str(wrapper, TEMPLATE_PATH_KEY, HTML_COMMENT_VOTE_WRAPPER);
    add_map_value_str(wrapper, COMMENT_ID_KEY, comment_id);
    add_map_value_str(wrapper, COMMENT_POINTS_KEY, points);

    // check if client user voted on this post
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

    // add post info map to vote wrapper map
    add_map_value_hm(wrapper, COMMENT_KEY, comment_info->hm);
    comment_info->hm = NULL;
    ks_list_add(comment_wrappers, ks_datacont_new(wrapper, KS_HASHMAP, 1));
  }
  ks_iterator_delete(iter);
  ks_list_delete(comments);

  return comment_wrappers;
}


struct response* get_user(const char* user_name, const struct auth_token* client_info)
{
  if (user_name == NULL || strlen(user_name) == 0)
  {
    return response_error(STAT404);
  }

  // get user info from DB
  ks_hashmap* page_data;
  if ((page_data = query_user_by_name(user_name)) == NULL)
  {
    // user not found
    return response_error(STAT404);
  }
  add_map_value_str(page_data, TEMPLATE_PATH_KEY, HTML_USER);

  // get user posts
  ks_list* posts;
  if ((posts = get_user_posts(user_name, client_info)) != NULL)
  {
    add_map_value_ls(page_data, USER_POST_LIST_KEY, posts);
  }

  // get user comments
  ks_list* comments;
  if ((comments = get_user_comments(user_name, client_info)) != NULL)
  {
    add_map_value_ls(page_data, USER_COMMENT_LIST_KEY, comments);
  }

  // add css visibility values for user options
  char* edit_vis = "hidden";
  char* logout_vis = "hidden";
  char* delete_vis = "hidden";

  if (client_info != NULL)
  {
    ks_hashmap* admin_info;
    if (strcmp(user_name, client_info->user_name) == 0)
    {
      edit_vis = "hidden";
      logout_vis = "hidden";
      delete_vis = "hidden";
    }
    else if ((admin_info = query_administrator_by_user_id(client_info->user_id)) != NULL)
    {
      ks_hashmap_delete(admin_info);
      delete_vis = "hidden";
    }
  }

  add_map_value_str(page_data, EDIT_OPTION_VISIBILITY_KEY, edit_vis);
  add_map_value_str(page_data, LOGOUT_OPTION_VISIBILITY_KEY, logout_vis);
  add_map_value_str(page_data, DELETE_OPTION_VISIBILITY_KEY, delete_vis);
 
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

  return response_ok(content);
}
