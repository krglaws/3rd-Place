#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <log_manager.h>
#include <util.h>
#include <auth_manager.h>
#include <sql_wrapper.h>
#include <http_get.h>
#include <get_post.h>


char* fill_post_info(char* template, const char* post_id, const struct auth_token* client_entry)
{
  // query post info
  char* query_fmt = QUERY_POST_BY_ID;
  char query[strlen(query_fmt) + strlen(post_id) + 1];
  sprintf(query, query_fmt, post_id);
  ks_list** result = query_database_ls(query);

  if (result[0] == NULL)
  {
    // post not found
    free(template);
    delete_query_result(result);
    return NULL;
  }

  // NOTE:
  // might remove this check
  if (result[1] != NULL)
  {
    free(template);
    delete_query_result(result);
    log_crit("fill_post_info(): multiple posts with id %s", post_id);
  }

  ks_list* post_info = result[0];

  // fill in post info
  if ((template = replace(template, "{DATE_POSTED}", ks_list_get(post_info, SQL_FIELD_POST_DATE_POSTED)->cp)) == NULL ||
      (template = replace(template, "{USER_NAME}", ks_list_get(post_info, SQL_FIELD_POST_AUTHOR_NAME)->cp)) == NULL ||
      (template = replace(template, "{USER_NAME}", ks_list_get(post_info, SQL_FIELD_POST_AUTHOR_NAME)->cp)) == NULL ||
      (template = replace(template, "{COMMUNITY_NAME}", ks_list_get(post_info, SQL_FIELD_POST_COMMUNITY_NAME)->cp)) == NULL ||
      (template = replace(template, "{COMMUNITY_NAME}", ks_list_get(post_info, SQL_FIELD_POST_COMMUNITY_NAME)->cp)) == NULL ||
      (template = replace(template, "{POST_TITLE}", ks_list_get(post_info, SQL_FIELD_POST_TITLE)->cp)) == NULL ||
      (template = replace(template, "{POST_BODY}", ks_list_get(post_info, SQL_FIELD_POST_BODY)->cp)) == NULL)
  {
    delete_query_result(result);
    return NULL;
  }

  delete_query_result(result);
  return template;
} // end fill_post_info()


char* fill_post_comment_template(char* template, const ks_list* comment_info, const struct auth_token* client_info)
{
  const char* comment_id = ks_list_get(comment_info, SQL_FIELD_COMMENT_ID)->cp;
  const char* comment_author = ks_list_get(comment_info, SQL_FIELD_COMMENT_AUTHOR_NAME)->cp;
  const char* comment_body = ks_list_get(comment_info, SQL_FIELD_COMMENT_BODY)->cp;
  const char* comment_points = ks_list_get(comment_info, SQL_FIELD_COMMENT_POINTS)->cp;
  const char* date_posted = ks_list_get(comment_info, SQL_FIELD_COMMENT_DATE_POSTED)->cp;
  const char* post_id = ks_list_get(comment_info, SQL_FIELD_COMMENT_POST_ID)->cp;
  const char* post_title = ks_list_get(comment_info, SQL_FIELD_COMMENT_POST_TITLE)->cp;
  const char* community_name = ks_list_get(comment_info, SQL_FIELD_COMMENT_COMMUNITY_NAME)->cp;

  char* upvote_css_class = "upvote_notclicked";
  char* downvote_css_class = "downvote_notclicked";

  // check if client has voted on this post
  if (client_info)
  {
    enum vote_type vt = check_for_vote(COMMENT_VOTE, comment_id, client_info->user_id);
    upvote_css_class = vt == UPVOTE ? "upvote_clicked" : upvote_css_class;
    downvote_css_class = vt == DOWNVOTE ? "downovte_clicked" : downvote_css_class;
  }

  // fill in template
  if ((template = replace(template, "{UPVOTE_CLICKED}", upvote_css_class)) == NULL ||
      (template = replace(template, "{ITEM_ID}", comment_id)) == NULL ||
      (template = replace(template, "{ITEM_POINTS}", comment_points)) == NULL ||
      (template = replace(template, "{DOWNVOTE_CLICKED}", downvote_css_class)) == NULL ||
      (template = replace(template, "{ITEM_ID}", comment_id)) == NULL ||
      (template = replace(template, "{DATE_POSTED}", date_posted)) == NULL ||
      (template = replace(template, "{USER_NAME}", comment_author)) == NULL ||
      (template = replace(template, "{USER_NAME}", comment_author)) == NULL ||
      (template = replace(template, "{COMMENT_BODY}", comment_body)) == NULL)
  {
    return NULL;
  }

  return template; 
} // end fill_post_comment_template()


char* fill_post_comments(char* template, const char* post_id, const struct auth_token* client_info)
{
  // prepare query string
  char* query_fmt = QUERY_COMMENTS_BY_POSTID;
  char query[strlen(query_fmt) + strlen(post_id) + 1];
  sprintf(query, query_fmt, post_id);

  // grab user comment ks_list from database
  ks_list** comments = query_database_ls(query);

  // load post comment template
  char* comment_template;
  if ((comment_template = load_vote_wrapper("comment", HTML_POST_COMMENT)) == NULL)
  {
    // failed to load template
    delete_query_result(comments);
    free(template);
    return NULL;
  }

  int comment_template_len = strlen(comment_template);

  // iterate over each comment
  for (int i = 0; comments[i] != NULL; i++)
  {
    // copy comment string instead of read it from disk every loop
    char* comment_template_copy = malloc((comment_template_len + 1) * sizeof(char));
    memcpy(comment_template_copy, comment_template, comment_template_len + 1);

    // current comment (field ks_list)
    ks_list* c = comments[i];

    // fill in comment template
    if ((comment_template_copy = fill_post_comment_template(comment_template_copy, c, client_info)) == NULL ||
        (template = replace(template, "{NEXT_ITEM}", comment_template_copy)) == NULL)
    {
      if (comment_template_copy != NULL)
      {
        free(comment_template_copy);
      }
      if (template != NULL)
      {
        free(template);
        template = NULL;
      }
      break;
    }

    free(comment_template_copy);
  }

  // remove last template string
  if (template != NULL)
  {
    template = replace(template, "{NEXT_ITEM}", "");
  }

  // cleanup
  delete_query_result(comments);
  free(comment_template);

  return template;
} // end fill_post_comments()


char* get_post(const char* post_id, const struct auth_token* client_info)
{
  if (post_id == NULL || strlen(post_id) == 0)
  {
    return NULL;
  }

  // load post template
  char* post_html;
  if ((post_html = load_file(HTML_POST)) == NULL)
  {
    // set error flag
    set_error(INTERNAL);
    return NULL;
  }
  
  // fill in post template
  if ((post_html = fill_post_info(post_html, post_id, client_info)) == NULL ||
      (post_html = fill_post_comments(post_html, post_id, client_info)) == NULL)
  {
    return NULL;
  }

  // load main template
  char* main_html;
  if ((main_html = load_file(HTML_MAIN)) == NULL)
  {
    // set internal error flag
    set_error(INTERNAL);
    free(post_html);
    return NULL;
  }

  // fill in main template
  if ((main_html = replace(main_html, "{STYLE}", CSS_POST)) == NULL ||
      (main_html = replace(main_html, "{SCRIPT}", "")) == NULL ||
      (main_html = fill_nav_login(main_html, client_info)) == NULL ||
      (main_html = replace(main_html, "{PAGE_BODY}", post_html)) == NULL)
  {
    free(post_html);
    return NULL;
  }

  free(post_html);
  return main_html;
} // end get_post()
