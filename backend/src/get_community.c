#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <log_manager.h>
#include <load_file.h>
#include <sql_manager.h>
#include <http_get.h>
#include <get_community.h>


char* fill_community_info(char* template, const char* community_name)
{
  // prepare query string
  char* query_fmt = QUERY_COMMUNITY_BY_NAME;
  char query[strlen(query_fmt) + strlen(community_name) + 1];
  sprintf(query, query_fmt, community_name);

  ks_list* result = query_database(query);
  ks_datacont* row0 = ks_list_get(result, 0);
  ks_datacont* row1 = ks_list_get(result, 1);

  if (row0 == NULL)
  {
    // community not found
    free(template);
    ks_list_delete(result);
    return NULL;
  }

  // NOTE:
  // might remove this check at some point
  if (row1 != NULL)
  {
    free(template);
    ks_list_delete(result);
    log_crit("fill_community_info(): multiple communities with name '%s'", community_name);
  }

  ks_list* community_info = row0->ls;

  const char* community_date_created = ks_list_get(community_info, SQL_FIELD_COMMUNITY_DATE_CREATED)->cp;
  const char* community_about = ks_list_get(community_info, SQL_FIELD_COMMUNITY_ABOUT)->cp;

  // fill in community info
  if ((template = replace(template, "{COMMUNITY_NAME}", community_name)) == NULL ||
      (template = replace(template, "{DATE_CREATED}", community_date_created)) == NULL ||
      (template = replace(template, "{COMMUNITY_ABOUT}", community_about)) == NULL)
  {
    ks_list_delete(result);
    return NULL;
  }

  ks_list_delete(result);
  return template;
} // end fill_community_info


char* fill_community_post_template(char* template, const ks_list* post_info, const struct auth_token* client_info)
{
  const char* post_id = ks_list_get(post_info, SQL_FIELD_POST_ID)->cp;
  const char* post_author = ks_list_get(post_info, SQL_FIELD_POST_AUTHOR_NAME)->cp;
  const char* post_title = ks_list_get(post_info, SQL_FIELD_POST_TITLE)->cp;
  const char* post_body = ks_list_get(post_info, SQL_FIELD_POST_BODY)->cp;
  const char* post_points = ks_list_get(post_info, SQL_FIELD_POST_POINTS)->cp;
  const char* date_posted = ks_list_get(post_info, SQL_FIELD_POST_DATE_POSTED)->cp;

  char* upvote_css_class = "upvote_notclicked";
  char* downvote_css_class = "downvote_notclicked";

  // check if client has voted on this post
  if (client_info != NULL)
  {
    enum vote_type vt = check_for_vote(POST_VOTE, post_id, client_info->user_id);
    upvote_css_class = vt == UPVOTE ? "upvote_clicked" : upvote_css_class;
    downvote_css_class = vt == DOWNVOTE ? "downovte_clicked" : downvote_css_class;
  }

  // limit post body to 64 chars in template
  char body[65];
  int end = sprintf(body, "%61s", post_body);
  memcpy(body + end, "...", 3);
  body[end+3] = '\0';
  template = replace(template, "{POST_BODY}", body);

  // fill template
  if ((template = replace(template, "{UPVOTE_CLICKED}", upvote_css_class)) == NULL ||
      (template = replace(template, "{ITEM_ID}", post_id)) == NULL ||
      (template = replace(template, "{ITEM_POINTS}", post_points)) == NULL ||
      (template = replace(template, "{DOWNVOTE_CLICKED}", downvote_css_class)) == NULL ||
      (template = replace(template, "{ITEM_ID}", post_id)) == NULL ||
      (template = replace(template, "{DATE_POSTED}", date_posted)) == NULL ||
      (template = replace(template, "{USER_NAME}", post_author)) == NULL ||
      (template = replace(template, "{USER_NAME}", post_author)) == NULL ||
      (template = replace(template, "{POST_ID}", post_id)) == NULL ||
      (template = replace(template, "{POST_TITLE}", post_title)) == NULL)
  {
    return NULL;
  }

  return template;
} // end fill_community_post_template()


char* fill_community_posts(char* template, const char* community_name, const struct auth_token* client_info)
{
  // prepare query string
  char* query_fmt = QUERY_POSTS_BY_COMMUNITY_NAME;
  char query[strlen(query_fmt) + strlen(community_name) + 1];
  sprintf(query, query_fmt, community_name);

  // grab posts belonging to this community
  ks_list* posts = query_database(query);

  // load post template
  char* post_template;
  if ((post_template = load_vote_wrapper("post", HTML_COMMUNITY_POST)) == NULL)
  {
    ks_list_delete(posts);
    free(template);
    return NULL;
  }

  int post_template_len = strlen(post_template);

  // iterate over each post
  int num_posts = ks_list_length(posts);
  for (int i = 0; i < num_posts; i++)
  {
    // copy template string instead of read it from disk every loop
    char* post_template_copy = malloc((post_template_len + 1) * sizeof(char));
    memcpy(post_template_copy, post_template, post_template_len + 1);

    // current post
    ks_list* p = ks_list_get(posts, i)->ls;

    if ((post_template_copy = fill_community_post_template(post_template_copy, p, client_info)) == NULL ||
        (template = replace(template, "{NEXT_ITEM}", post_template_copy)) == NULL)
    {
      if (post_template_copy != NULL)
      {
        free(post_template_copy);
      }
      if (template != NULL)
      {
        free(template);
        template = NULL;
      }
      break;
    }

    free(post_template_copy);
  }

  // remove last template string
  if (template != NULL)
  {
    template = replace(template, "{NEXT_ITEM}", "");
  }

  // cleanup
  ks_list_delete(posts);
  free(post_template);

  // remove trailing template string
  return template;
} // end fill_community_posts()


char* get_community(const char* community_name, const struct auth_token* client_info)
{
  if (community_name == NULL || strlen(community_name) == 0)
  {
    return NULL;
  }

  // load community template
  char* community_html;
  if ((community_html = load_file(HTML_COMMUNITY)) == NULL)
  {
    // set error flag
    set_error(INTERNAL);
    return NULL;
  }

  // fill in community template
  if ((community_html = fill_community_info(community_html, community_name)) == NULL ||
      (community_html = fill_community_posts(community_html, community_name, client_info)) == NULL)
  {
    return NULL;
  }

  // load main html
  char* main_html;
  if ((main_html = load_file(HTML_MAIN)) == NULL)
  {
    // set error flag
    set_error(INTERNAL);
    free(community_html);
    return NULL;
  }

  // fill in main template
  if ((main_html = replace(main_html, "{STYLE}", CSS_COMMUNITY)) == NULL ||
      (main_html = replace(main_html, "{SCRIPT}", JS_COMMUNITY)) == NULL ||
      (main_html = fill_nav_login(main_html, client_info)) == NULL ||
      (main_html = replace(main_html, "{PAGE_BODY}", community_html)) == NULL)
  {
    free(community_html);
    return NULL;
  }

  free(community_html);
  return main_html;
} // end get_community()
