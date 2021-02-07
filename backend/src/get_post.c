#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <kylestructs.h>

#include <templating.h>
#include <senderr.h>
#include <string_map.h>
#include <log_manager.h>
#include <load_file.h>
#include <auth_manager.h>
#include <sql_manager.h>
#include <http_get.h>
#include <get_post.h>


static ks_list* get_post_comments(const char* post_id, const struct auth_token* client_info)
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
    const char* comment_id = get_map_value(comment_info->hm, FIELD_COMMENT_ID)->cp;
    const char* points = get_map_value(comment_info->hm, FIELD_COMMENT_POINTS)->cp;

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
    return senderr(ERR_NOT_FOUND);
  }

  // get post from DB
  ks_hashmap* post_info;
  if ((post_info = get_post_info(post_id)) == NULL)
  {
    return senderr(ERR_NOT_FOUND);
  }

  char* upvote_class = UPVOTE_NOTCLICKED_STATE;
  char* downvote_class = DOWNVOTE_NOTCLICKED_STATE;
  if (client_info != NULL)
  {
    enum vote_type vt = check_for_vote(POST_ITEM, post_id, client_info->user_id);
    upvote_class = vt == UPVOTE ? UPVOTE_CLICKED_STATE : upvote_class;
    downvote_class = vt == DOWNVOTE ? DOWNVOTE_CLICKED_STATE : downvote_class;
  }

  add_map_value_str(post_info, UPVOTE_CLICKED_KEY, upvote_class);
  add_map_value_str(post_info, DOWNVOTE_CLICKED_KEY, downvote_class);
  add_map_value_str(post_info, TEMPLATE_PATH_KEY, HTML_POST);

  // get post comments
  ks_list* comments;
  if ((comments = get_post_comments(post_id, client_info)) == NULL)
  {
    add_map_value_str(post_info, POST_COMMENT_LIST_KEY, " ");
  }
  else
  {
    add_map_value_ls(post_info, POST_COMMENT_LIST_KEY, comments);
  }

  // put page data together
  ks_hashmap* page_data = ks_hashmap_new(KS_CHARP, 8);
  add_map_value_hm(page_data, PAGE_CONTENT_KEY, post_info);
  add_map_value_str(page_data, STYLE_PATH_KEY, CSS_POST);
  add_map_value_str(page_data, SCRIPT_PATH_KEY, " ");
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
