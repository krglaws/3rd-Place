#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <util.h>
#include <http_get.h>
#include <sql_wrapper.h>
#include <get_user.h>


char* fill_user_info(char* template, const char* user_name)
{
  // prepare query string
  char* query_fmt = QUERY_USER_BY_UNAME;
  char query[strlen(user_name) + strlen(query_fmt) + 1];
  sprintf(query, query_fmt, user_name);

  list** result = query_database_ls(query);

  if (result[0] == NULL)
  {
    fprintf(stderr, "user %s not found\n", user_name);
    free(template);
    free(result);
    return NULL;
  }

#ifdef DEBUG
  // make sure we got exactly ONE entry
  if (result[1] != NULL)
  {
    fprintf(stderr, "fill_user_info(): multiple users with name %s\n", user_name);
    exit(EXIT_FAILURE);
  }
#endif

  list* user_info = result[0];
  free(result);

  const char* user_points = list_get(user_info, SQL_FIELD_USER_POINTS)->cp;
  const char* user_posts = list_get(user_info, SQL_FIELD_USER_POSTS)->cp;
  const char* user_comments = list_get(user_info, SQL_FIELD_USER_COMMENTS)->cp;
  const char* user_date_joined = list_get(user_info, SQL_FIELD_USER_DATE_JOINED)->cp;
  const char* user_about = list_get(user_info, SQL_FIELD_USER_ABOUT)->cp;

  // fill in user info
  if ((template = replace(template, "{USER_NAME}", user_name)) == NULL ||
      (template = replace(template, "{USER_POINTS}", user_points)) == NULL ||
      (template = replace(template, "{USER_POSTS}", user_posts)) == NULL ||
      (template = replace(template, "{USER_COMMENTS}", user_comments)) == NULL ||
      (template = replace(template, "{USER_BDAY}", user_date_joined)) == NULL ||
      (template = replace(template, "{USER_ABOUT}", user_about)) == NULL)
  {
    list_delete(user_info);
    return NULL;
  }

  list_delete(user_info);
  return template;
} // end fill_user_info()


char* fill_user_post_template(char* template, const list* post_info, const struct token_entry* client_info)
{
  const char* post_id = list_get(post_info, SQL_FIELD_POST_ID)->cp;
  const char* post_title = list_get(post_info, SQL_FIELD_POST_TITLE)->cp;
  const char* post_body = list_get(post_info, SQL_FIELD_POST_BODY)->cp;
  const char* post_points = list_get(post_info, SQL_FIELD_POST_POINTS)->cp;
  const char* date_posted = list_get(post_info, SQL_FIELD_POST_DATE_POSTED)->cp;
  const char* community_name = list_get(post_info, SQL_FIELD_POST_COMMUNITY_NAME)->cp;

  char* upvote_css_class = "upvote_notclicked";
  char* downvote_css_class = "downvote_notclicked";

  // check if client has voted on this post
  if (client_info)
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

  // fill in template
  if ((template = replace(template, "{UPVOTE_CLICKED}", upvote_css_class)) == NULL ||
      (template = replace(template, "{ITEM_ID}", post_id)) == NULL ||
      (template = replace(template, "{ITEM_POINTS}", post_points)) == NULL ||
      (template = replace(template, "{DOWNVOTE_CLICKED}", downvote_css_class)) == NULL ||
      (template = replace(template, "{ITEM_ID}", post_id)) == NULL ||
      (template = replace(template, "{DATE_POSTED}", date_posted)) == NULL ||
      (template = replace(template, "{POST_ID}", post_id)) == NULL ||
      (template = replace(template, "{POST_TITLE}", post_title)) == NULL ||
      (template = replace(template, "{COMMUNITY_NAME}", community_name)) == NULL ||
      (template = replace(template, "{COMMUNITY_NAME}", community_name)) == NULL ||
      (template = replace(template, "{POST_BODY}", body)) == NULL)
  {
    return NULL;
  }

  return template;
} // end fill_user_post_template


char* fill_user_posts(char* template, char* user_name, const struct token_entry* client_info)
{
  // query user posts
  char* post_query_fmt = QUERY_POSTS_BY_UNAME;
  char post_query[strlen(post_query_fmt) + strlen(user_name) + 1];
  sprintf(post_query, post_query_fmt, user_name);
  list** posts = query_database_ls(post_query);

  // load user post template
  char* post_template;
  if ((post_template = load_vote_wrapper("post", HTML_USER_POST)) == NULL)
  {
    for (int i = 0; posts[i] != NULL; i++)
    {
      list_delete(posts[i]);
    }
    free(posts);
    free(template);
    return NULL;
  }

  int post_template_len = strlen(post_template);

  // iterate through each post
  for (int i = 0; posts[i] != NULL; i++)
  {
    list* p = posts[i];

    // copy post template string instead of read it from disk every loop
    char* post_template_copy = malloc((post_template_len + 1) * sizeof(char));
    memcpy(post_template_copy, post_template, post_template_len + 1);

    // fill in post template
    if ((post_template_copy = fill_user_post_template(post_template_copy, p, client_info)) == NULL ||
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
  for (int i = 0; posts[i] != NULL; i++)
  {
    list_delete(posts[i]);
  }
  free(posts);
  free(post_template);

  return template;
} // end fill_user_posts()


char* fill_user_comment_template(char* template, const list* comment_info, const struct token_entry* client_info)
{
  const char* comment_id = list_get(comment_info, SQL_FIELD_COMMENT_ID)->cp;
  const char* comment_body = list_get(comment_info, SQL_FIELD_COMMENT_BODY)->cp;
  const char* comment_points = list_get(comment_info, SQL_FIELD_COMMENT_POINTS)->cp;
  const char* date_posted = list_get(comment_info, SQL_FIELD_COMMENT_DATE_POSTED)->cp;
  const char* post_id = list_get(comment_info, SQL_FIELD_COMMENT_POST_ID)->cp;
  const char* post_title = list_get(comment_info, SQL_FIELD_COMMENT_POST_TITLE)->cp;
  const char* community_name = list_get(comment_info, SQL_FIELD_COMMENT_COMMUNITY_NAME)->cp;

  char* upvote_css_class = "upvote_notclicked";
  char* downvote_css_class = "downvote_notclicked";

  // check if client has voted on this post
  if (client_info != NULL)
  {
    enum vote_type vt = check_for_vote(COMMENT_VOTE, comment_id, client_info->user_id);
    upvote_css_class = vt == UPVOTE ? "upvote_clicked" : upvote_css_class;
    downvote_css_class = vt == DOWNVOTE ? "downovte_clicked" : downvote_css_class;
  }

  // limit comment body to 64 chars in template
  char body[65];
  int end = sprintf(body, "%61s", comment_body);
  memcpy(body + end, "...", 3);
  body[end+3] = '\0';
  template = replace(template, "{COMMENT_BODY}", body);

  // fill in template
  if ((template = replace(template, "{UPVOTE_CLICKED}", upvote_css_class)) == NULL ||
      (template = replace(template, "{ITEM_ID}", comment_id)) == NULL ||
      (template = replace(template, "{ITEM_POINTS}", comment_points)) == NULL ||
      (template = replace(template, "{DOWNVOTE_CLICKED}", downvote_css_class)) == NULL ||
      (template = replace(template, "{ITEM_ID}", comment_id)) == NULL ||
      (template = replace(template, "{DATE_POSTED}", date_posted)) == NULL ||
      (template = replace(template, "{POST_ID}", post_id)) == NULL ||
      (template = replace(template, "{COMMENT_ID}", comment_id)) == NULL ||
      (template = replace(template, "{POST_ID}", post_id)) == NULL ||
      (template = replace(template, "{POST_TITLE}", post_title)) == NULL ||
      (template = replace(template, "{COMMUNITY_NAME}", community_name)) == NULL ||
      (template = replace(template, "{COMMUNITY_NAME}", community_name)) == NULL)
  {
    return NULL;
  }

  return template; 
} // end fill_user_comment_template()


char* fill_user_comments(char* template, const char* user_name, const struct token_entry* client_info)
{
  // query user comments
  char* query_fmt = QUERY_COMMENTS_BY_UNAME;
  char query[strlen(query_fmt) + strlen(user_name) + 1];
  sprintf(query, query_fmt, user_name);
  list** comments = query_database_ls(query);

  // load comment template
  char* comment_template;
  if ((comment_template = load_vote_wrapper("comment", HTML_USER_COMMENT)) == NULL)
  {
    for (int i = 0; comments[i] != NULL; i++)
    {
      list_delete(comments[i]);
    }
    free(comments);
    free(template);
    return NULL;
  }

  int comment_template_len = strlen(comment_template);

  // iterate over each comment
  for (int i = 0; comments[i] != NULL; i++)
  {
    // current comment (list of fields)
    list* c = comments[i];

    // copy comment template string instead of read it from disk every loop
    char* comment_template_copy = malloc((comment_template_len + 1) * sizeof(char));
    memcpy(comment_template_copy, comment_template, comment_template_len + 1);

    // fill in comment template
    if ((comment_template_copy = fill_user_comment_template(comment_template_copy, c, client_info)) == NULL ||
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
  for (int i = 0; comments[i] != NULL; i++)
  {
    list_delete(comments[i]);
  }
  free(comments);
  free(comment_template);

  return template;
} // end fill_user_comments()


char* get_user(char* user_name, const struct token_entry* client_info)
{
  if (user_name == NULL || strlen(user_name) == 0)
  {
    return NULL;
  }

  // load user template
  char* user_html;
  if ((user_html = load_file(HTML_USER)) == NULL)
  {
    return NULL;
  } 

  // fill in user template
  if ((user_html = fill_user_info(user_html, user_name)) == NULL ||
      (user_html = fill_user_posts(user_html, user_name, client_info)) == NULL ||
      (user_html = fill_user_comments(user_html, user_name, client_info)) == NULL)
  {
    return NULL;
  }

  // load main template
  char* main_html;
  if ((main_html = load_file(HTML_MAIN)) == NULL)
  {
    free(user_html);
    return NULL;
  }

  // fill in main template
  if ((main_html = replace(main_html, "{STYLE}", CSS_USER)) == NULL ||
      (main_html = replace(main_html, "{SCRIPT}", JS_USER)) == NULL ||
      (main_html = fill_nav_login(main_html, client_info)) == NULL ||
      (main_html = replace(main_html, "{PAGE_BODY}", user_html)) == NULL)
  {
    free(user_html);
    return NULL;
  }

  free(user_html);
  return main_html;
} // end get_user()
