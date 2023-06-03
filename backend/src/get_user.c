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


static ks_list* get_user_posts(const char* user_id, const struct auth_token* client_info, const char* page_no, const char* page_size)
{
  // get user posts
  ks_list* posts;
  if ((posts = query_posts_by_author_id(user_id, page_no, page_size)) == NULL)
  {
    return NULL;
  }

  // list of vote wrappers
  ks_list* post_wrappers = ks_list_new();

  // iterate through each post
  ks_datacont* post_info;
  ks_iterator* iter = ks_iterator_new(posts, KS_LIST);
  while ((post_info = (ks_datacont*) ks_iterator_next(iter)) != NULL)
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


static ks_list* get_user_comments(const char* user_id, const struct auth_token* client_info, const char* page_no, const char* page_size)
{
  // get user comments
  ks_list* comments;
  if ((comments = query_comments_by_author_id(user_id, page_no, page_size)) == NULL)
  {
    return NULL;
  }

  // list of vote wrappers
  ks_list* comment_wrappers = ks_list_new();

  // iterate through each post
  ks_datacont* comment_info;
  ks_iterator* iter = ks_iterator_new(comments, KS_LIST);
  while ((comment_info = (ks_datacont*) ks_iterator_next(iter)) != NULL)
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


static ks_list* get_user_owner_list(const char* user_id)
{
  ks_list* owner_list;
  if ((owner_list = query_communities_by_owner_id(user_id)) == NULL)
  {
    return NULL;
  }

  const ks_datacont* curr;
  ks_iterator* iter = ks_iterator_new(owner_list, KS_LIST);
  while ((curr = ks_iterator_next(iter)) != NULL)
  {
    add_map_value_str(curr->hm, TEMPLATE_PATH_KEY, HTML_USER_OWNER_TITLE);
  }
  ks_iterator_delete(iter);

  return owner_list;
}


static ks_list* get_user_mod_list(const char* user_id)
{
  ks_list* mod_list;
  if ((mod_list = query_moderators_by_user_id(user_id)) == NULL)
  {
    return NULL;
  }

  const ks_datacont* curr;
  ks_iterator* iter = ks_iterator_new(mod_list, KS_LIST);
  while ((curr = ks_iterator_next(iter)) != NULL)
  {
    add_map_value_str(curr->hm, TEMPLATE_PATH_KEY, HTML_USER_MOD_TITLE);
  }
  ks_iterator_delete(iter);

  return mod_list;
}


struct response* get_user(const struct request* req)
{
  const char* user_name = get_map_value_str(req->query, "name");
  const char* user_id = get_map_value_str(req->query, "id");

  // get user info from DB
  ks_hashmap* page_data;
  if (user_id != NULL)
  {
    if ((page_data = query_user_by_id(user_id)) == NULL)
    {
      return response_error(STAT404);
    }
    user_name = get_map_value_str(page_data, FIELD_USER_NAME);
  }
  else if (user_name != NULL)
  {
    if ((page_data = query_user_by_name(user_name)) == NULL)
    {
      return response_error(STAT404);
    }
    user_id = get_map_value_str(page_data, FIELD_USER_ID);
  }
  else
  {
    return response_error(STAT404);
  }
  add_map_value_str(page_data, TEMPLATE_PATH_KEY, HTML_USER);

  const char *page_no = "1", *page_size = "10";

  const ks_datacont* val;
  if (val = get_map_value(req->query, "page_no"))
  {
    page_no = val->cp;
  }
  add_map_value_str(page_data, FEED_PAGE_NO_KEY, page_no);

  if (val = get_map_value(req->query, "page_size"))
  {
    page_size = val->cp;
  }
  add_map_value_str(page_data, FEED_PAGE_SIZE_KEY, page_size);

  // get user posts
  ks_list* posts;
  if ((posts = get_user_posts(user_id, req->client_info, page_no, page_size)) != NULL)
  {
    add_map_value_ls(page_data, USER_POST_LIST_KEY, posts);
  }
  else
  {
    add_map_value_str(page_data, USER_POST_LIST_KEY, "<p><em>Nothing to see here...</em></p>");
  }

  // get user comments
  ks_list* comments;
  if ((comments = get_user_comments(user_id, req->client_info, page_no, page_size)) != NULL)
  {
    add_map_value_ls(page_data, USER_COMMENT_LIST_KEY, comments);
  }
  else
  {
    add_map_value_str(page_data, USER_COMMENT_LIST_KEY, "<p><em>Nothing to see here...</em></p>");
  }

  // is user an admin?
  ks_hashmap* admin_info;
  if ((admin_info = query_administrator_by_user_id(user_id)) != NULL)
  {
    add_map_value_str(admin_info, TEMPLATE_PATH_KEY, HTML_USER_ADMIN_TITLE);
    add_map_value_hm(page_data, USER_ADMIN_KEY, admin_info);
  }

  // get owned community list
  ks_list* owner_list;
  if ((owner_list = get_user_owner_list(user_id)) != NULL)
  {
    add_map_value_ls(page_data, USER_OWNER_LIST_KEY, owner_list);
  }

  // get moderator list
  ks_list* mod_list;
  if ((mod_list = get_user_mod_list(user_id)) != NULL)
  {
    add_map_value_ls(page_data, USER_MOD_LIST_KEY, mod_list);
  }

  // add css visibility values for user options
  char* edit_vis = "hidden";
  char* logout_vis = "hidden";
  char* delete_vis = "hidden";
  char* sub_vis = "hidden";
  if (req->client_info != NULL)
  {
    ks_hashmap* user_admin_info;
    if (strcmp(user_name, req->client_info->user_name) == 0)
    {
      edit_vis = "visible";
      logout_vis = "visible";
      delete_vis = "visible";
      sub_vis = "visible";
    }

    else if ((user_admin_info = query_administrator_by_user_id(req->client_info->user_id)) != NULL)
    {
      ks_hashmap_delete(user_admin_info);
      delete_vis = "visible";
    }
  }
  add_map_value_str(page_data, EDIT_OPTION_VISIBILITY_KEY, edit_vis);
  add_map_value_str(page_data, LOGOUT_OPTION_VISIBILITY_KEY, logout_vis);
  add_map_value_str(page_data, DELETE_OPTION_VISIBILITY_KEY, delete_vis);
  add_map_value_str(page_data, SUBSCRIPTIONS_OPTION_VISIBILITY_KEY, sub_vis);
 
  // put page data together
  page_data = wrap_page_data(req->client_info, page_data);

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
