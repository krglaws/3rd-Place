
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <kylestructs.h>

#include <common.h>
#include <util.h>
#include <senderr.h>
#include <token_manager.h>
#include <sql_wrapper.h>
#include <http_get.h>


static list* recent_posts;


struct response* http_get(struct request* req)
{
  if (req == NULL) return NULL;

  /* what are we getting? */
  char* content_type = NULL;
  char* content = NULL;
  int content_length = 0;
  if (strstr(req->uri, ".css"))
  {
    content_type = TEXTCSS;
    if (content = load_file(req->uri))
    {
      content_length = strlen(content);
    }
  }
  else if (strstr(req->uri, ".js"))
  {
    content_type = APPJS;
    if (content = load_file(req->uri))
    {
      content_length = strlen(content);
    }
  }
  else if (strstr(req->uri, ".ico"))
  {
    struct file_str* ico;
    content_type = IMGICO;
    if (ico = load_file_str(req->uri))
    {
      content = ico->contents;
      content_length = ico->len;
      free(ico);
    }
  }
  else if (strstr(req->uri, "./u/"))
  {
    content_type = TEXTHTML;
    if (content = get_user(req->uri + 4, req->token))
    {
      content_length = strlen(content);
    }
  }
  else if (strstr(req->uri, "./c/"))
  {
    content_type = TEXTHTML;
    if (content = get_community(req->uri + 4, req->token))
    {
      content_length = strlen(content);
    }
  }
  else if (strstr(req->uri, "./p/"))
  {
    content_type = TEXTHTML;
    if (content = get_post(req->uri + 4, req->token))
    {
      content_length = strlen(content);
    }
  }

  /* do we have it? */
  if (content_type == NULL || content == NULL)
  {
    if (content != NULL)
    {
      free(content);
    }
    return senderr(NOT_FOUND);
  }

  /* prepare response object */
  struct response* resp = calloc(1, sizeof(struct response));
  resp->header = list_new();
  list_add(resp->header, datacont_new(STAT200, CHARP, strlen(STAT200)));
  list_add(resp->header, datacont_new(content_type, CHARP, strlen(content_type)));
 
  char contlenline[80];
  int len = sprintf(contlenline, "Content-Length: %d\n", content_length);

  list_add(resp->header, datacont_new(contlenline, CHARP, len));
  resp->content = content;
  resp->content_length = content_length;

  return resp;
}


char* replace(char* template, const char* this, const char* withthat)
{
  char* location;

  // check if 'this' is in template
  if ((location = strstr(template, this)) == NULL)
  {
    return template;
  }

  // figure out sizes
  int templen = strlen(template);
  int thislen = strlen(this);
  int withlen = strlen(withthat);
  int outputsize = (templen - thislen) + withlen + 1;

  // create output buffer
  char* output = malloc(outputsize);
  output[outputsize - 1] = '\0';

  // do the replacement
  int dist = (location - template); 
  memcpy(output, template, dist);
  memcpy(output + dist, withthat, withlen);
  memcpy(output + dist + withlen, template + dist + thislen, strlen(template + dist + thislen));

  free(template);

  return output;
}


char* fill_nav_login(char* template, char* token)
{
  char user_link[64];

  const char* client_uname = valid_token(token);

  if (client_uname != NULL)
  {
    sprintf(user_link, "/u/%s", client_uname);
    template = replace(template, "{USER_LINK}", user_link);
    template = replace(template, "{USER_NAME}", client_uname);
  }

  else
  {
    template = replace(template, "{USER_LINK}", "/signup");
    template = replace(template, "{USER_NAME}", "Sign Up");
  }

  return template;
}


char* fill_user_info(char* template, char* uname)
{
  // prepare query string
  char* query_fmt = QUERY_USER_BY_UNAME;
  char query[strlen(uname) + strlen(query_fmt) + 1];
  sprintf(query, query_fmt, uname);

  list** result = query_database_ls(query);

  // make sure we got exactly ONE entry
  if (result[0] == NULL)
  {
    fprintf(stderr, "user %s not found\n", uname);
    free(template);
    free(result);
    return NULL;
  }

  if (result[1] != NULL)
  {
    fprintf(stderr, "fill_user_info(): multiple users with name %s\n", uname);
    exit(EXIT_FAILURE);
  }
  list* user_info = result[0];
  free(result);

  // fill in user info
  template = replace(template, "{USER_NAME}", uname);
  template = replace(template, "{USER_POINTS}", list_get(user_info, SQL_FIELD_USER_POINTS)->cp);
  template = replace(template, "{USER_POSTS}", list_get(user_info, SQL_FIELD_USER_POSTS)->cp);
  template = replace(template, "{USER_COMMENTS}", list_get(user_info, SQL_FIELD_USER_COMMENTS)->cp);
  template = replace(template, "{USER_BDAY}", list_get(user_info, SQL_FIELD_USER_DATE_JOINED)->cp);
  template = replace(template, "{USER_ABOUT}", list_get(user_info, SQL_FIELD_USER_ABOUT)->cp);

  list_delete(user_info);

  return template;
}


char* fill_user_posts(char* template, char* uname)
{
  // prepare query string
  char* query_fmt = QUERY_POSTS_BY_UNAME;
  char query[strlen(query_fmt) + strlen(uname) + 1];
  sprintf(query, query_fmt, uname);

  // grab user post list from database
  list** posts = query_database_ls(query);

  // load post stub template and do replacements
  char* post_stub;
  if ((post_stub = load_file(HTML_USER_POST_STUB)) == NULL)
  {
    // failed to load post stub file
    fprintf(stderr, "fill_user_posts(): failed to load file %s\n", HTML_USER_POST_STUB);
    for (int i = 0; posts[i] != NULL; i++)
    {
      list_delete(posts[i]);
    }
    free(posts);
    free(template);
    return NULL;
  }

  int post_stub_len = strlen(post_stub);

  // iterate through each post
  for (int i = 0; posts[i] != NULL; i++)
  {
    // copy post stub string instead of read it from disk every loop
    char* post_stub_copy = malloc((post_stub_len + 1) * sizeof(char));
    memcpy(post_stub_copy, post_stub, post_stub_len + 1);

    // current post (list of fields)
    list* p = posts[i];
    post_stub_copy = replace(post_stub_copy, "{POST_ID}", list_get(p, SQL_FIELD_POST_ID)->cp);
    post_stub_copy = replace(post_stub_copy, "{POST_TITLE}", list_get(p, SQL_FIELD_POST_TITLE)->cp);
    post_stub_copy = replace(post_stub_copy, "{COMMUNITY_NAME}", list_get(p, SQL_FIELD_POST_COMMUNITY_ID)->cp);
    post_stub_copy = replace(post_stub_copy, "{COMMUNITY_NAME}", list_get(p, SQL_FIELD_POST_COMMUNITY_NAME)->cp);

    // limit post body to 64 chars in stub
    char body[65];
    int end = sprintf(body, "%61s", list_get(p, SQL_FIELD_POST_BODY)->cp);
    memcpy(body + end, "...", 3);
    body[end+3] = '\0';
    post_stub_copy = replace(post_stub_copy, "{POST_BODY}", body);

    // copy stub into main template
    template = replace(template, "{POST_STUB}", post_stub_copy);

    free(post_stub_copy);
    list_delete(p);
  }

  free(posts);
  free(post_stub);

  // remove trailing template string
  return replace(template, "{POST_STUB}", "");
}


char* fill_user_comments(char* template, char* uname)
{
  // prepare query string
  char* query_fmt = QUERY_COMMENTS_BY_UNAME;
  char query[strlen(query_fmt) + strlen(uname) + 1];
  sprintf(query, query_fmt, uname);

  // grab user comment list from database
  list** comments = query_database_ls(query);

  // load comment stub template and do replacements
  char* comment_stub;
  if ((comment_stub = load_file(HTML_USER_COMMENT_STUB)) == NULL)
  {
    // failed to load comment stub file
    fprintf(stderr, "fill_user_comments(): failed to load file %s\n", HTML_USER_COMMENT_STUB);
    for (int i = 0; comments[i] != NULL; i++)
    {
      list_delete(comments[i]);
    }
    free(comments);
    free(template);
    return NULL;
  }

  int comment_stub_len = strlen(comment_stub);

  // iterate over each comment
  for (int i = 0; comments[i] != NULL; i++)
  {
    // current comment (list of fields)
    list* c = comments[i];

    // copy comment stub string instead of read it from disk every loop
    char* comment_stub_copy = malloc((comment_stub_len + 1) * sizeof(char));
    memcpy(comment_stub_copy, comment_stub, comment_stub_len + 1);

    // load comment_stub template and do replacements
    comment_stub_copy = replace(comment_stub_copy, "{POST_ID}", list_get(c, SQL_FIELD_COMMENT_POST_ID)->cp);
    comment_stub_copy = replace(comment_stub_copy, "{COMMENT_ID}", list_get(c, SQL_FIELD_COMMENT_ID)->cp);

    // limit comment body to 64 chars in stub
    char body[65];
    int end = sprintf(body, "%61s", list_get(c, SQL_FIELD_COMMENT_BODY)->cp);
    memcpy(body + end, "...", 3);
    body[end+3] = '\0';
    comment_stub_copy = replace(comment_stub_copy, "{COMMENT_BODY}", body);
    comment_stub_copy = replace(comment_stub_copy, "{POST_TITLE}", list_get(c, SQL_FIELD_COMMENT_POST_TITLE)->cp);
    comment_stub_copy = replace(comment_stub_copy, "{POST_ID}", list_get(c, SQL_FIELD_COMMENT_POST_ID)->cp);
    comment_stub_copy = replace(comment_stub_copy, "{COMMUNITY_NAME}", list_get(c, SQL_FIELD_COMMENT_COMMUNITY_NAME)->cp);
    comment_stub_copy = replace(comment_stub_copy, "{COMMUNITY_NAME}", list_get(c, SQL_FIELD_COMMENT_COMMUNITY_NAME)->cp);

    // copy stub into main template
    template = replace(template, "{COMMENT_STUB}", comment_stub_copy);

    free(comment_stub_copy);
    list_delete(c);
  }

  free(comments);
  free(comment_stub);

  // remove trailing template string
  return replace(template, "{COMMENT_STUB}", "");
}


char* get_user(char* uname, char* token)
{
  if (uname == NULL || strlen(uname) == 0)
  {
    return NULL;
  }

  // load user template
  char* user_html;
  if ((user_html = load_file(HTML_USER)) == NULL)
  {
    fprintf(stderr, "get_user(): failed to load %s\n", HTML_USER);
    return NULL;
  }

  // fill in user info
  if ((user_html = fill_user_info(user_html, uname)) == NULL)
  {
    // user does not exist
    return NULL;
  }

  user_html = fill_user_posts(user_html, uname);
  user_html = fill_user_comments(user_html, uname);

  // load main template
  char* main_html;
  if ((main_html = load_file(HTML_MAIN)) == NULL)
  {
    fprintf(stderr, "get_user(): failed to load %s\n", HTML_MAIN);
    free(user_html);
    return NULL;
  }

  // insert header info
  main_html = replace(main_html, "{STYLE}", CSS_USER);
  main_html = replace(main_html, "{SCRIPT}", JS_USER);

  // is client logged in?
  main_html = fill_nav_login(main_html, token);

  // insert user html into main
  main_html = replace(main_html, "{PAGE_BODY}", user_html);
  free(user_html);

  return main_html;
}


char* fill_post_info(char* template, char* postid)
{
  // prepare query string
  char* query_fmt = QUERY_POST_BY_UUID;
  char query[strlen(query_fmt) + strlen(postid) + 1];
  sprintf(query, query_fmt, postid);

  list** result = query_database_ls(query);

  // make sure we got exactly ONE entry
  if (result[0] == NULL)
  {
    fprintf(stderr, "post %s not found\n", postid);
    free(template);
    free(result);
    return NULL;
  }

  if (result[1] != NULL)
  {
    fprintf(stderr, "fill_post_info(): multiple posts with id %s\n", postid);
    exit(EXIT_FAILURE);
  }
  list* post_info = result[0];
  free(result);

  // fill in post info
  template = replace(template, "{USER_NAME}", list_get(post_info, SQL_FIELD_POST_AUTHOR_NAME)->cp);
  template = replace(template, "{USER_NAME}", list_get(post_info, SQL_FIELD_POST_AUTHOR_NAME)->cp);
  template = replace(template, "{COMMUNITY_NAME}", list_get(post_info, SQL_FIELD_POST_COMMUNITY_NAME)->cp);
  template = replace(template, "{COMMUNITY_NAME}", list_get(post_info, SQL_FIELD_POST_COMMUNITY_NAME)->cp);
  template = replace(template, "{POST_TITLE}", list_get(post_info, SQL_FIELD_POST_TITLE)->cp);
  template = replace(template, "{POST_BODY}", list_get(post_info, SQL_FIELD_POST_BODY)->cp);

  list_delete(post_info);

  return template;
}


char* fill_post_comments(char* template, char* postid)
{
  // prepare query string
  char* query_fmt = QUERY_COMMENTS_BY_POSTID;
  char query[strlen(query_fmt) + strlen(postid) + 1];
  sprintf(query, query_fmt, postid);

  // grab user comment list from database
  list** comments = query_database_ls(query);

  // load comment stub template
  char* comment_stub;
  if ((comment_stub = load_file(HTML_POST_COMMENT)) == NULL)
  {
    // failed to load comment stub
    fprintf(stderr, "fill_post_comments(): failed to load file %s\n", HTML_POST_COMMENT);
    for (int i = 0; comments[i] != NULL; i++)
    {
      list_delete(comments[i]);
    }
    free(comments); 
    free(template);
    return NULL;
  }

  int comment_stub_len = strlen(comment_stub);

  // iterate over each comment
  for (int i = 0; comments[i] != NULL; i++)
  {
    // copy comment stub string instead of read it from disk every loop
    char* comment_stub_copy = malloc((comment_stub_len + 1) * sizeof(char));
    memcpy(comment_stub_copy, comment_stub, comment_stub_len + 1);

    // current comment (list of fields)
    list* c = comments[i];
    comment_stub_copy = replace(comment_stub_copy, "{USER_NAME}", list_get(c, SQL_FIELD_COMMENT_AUTHOR_NAME)->cp);
    comment_stub_copy = replace(comment_stub_copy, "{USER_NAME}", list_get(c, SQL_FIELD_COMMENT_AUTHOR_NAME)->cp);
    comment_stub_copy = replace(comment_stub_copy, "{COMMENT_BODY}", list_get(c, SQL_FIELD_COMMENT_BODY)->cp);

    template = replace(template, "{COMMENT}", comment_stub_copy);

    free(comment_stub_copy);
    list_delete(c);
  }

  free(comments);
  free(comment_stub);

  // remove trailing template string
  return replace(template, "{COMMENT}", "");
}


char* get_post(char* postid, char* token)
{
  if (postid == NULL || strlen(postid) == 0)
  {
    return NULL;
  }

  // load post template
  char* post_html;
  if ((post_html = load_file(HTML_POST)) == NULL)
  {
    fprintf(stderr, "get_post(): failed to load %s\n", HTML_POST);
    return NULL;
  }

  // fill in post info
  if ((post_html = fill_post_info(post_html, postid)) == NULL)
  {
    // post does not exist
    return NULL;
  }

  post_html = fill_post_comments(post_html, postid);

  char* main_html;
  if ((main_html = load_file(HTML_MAIN)) == NULL)
  {
    fprintf(stderr, "get_post() failed to load %s\n", HTML_MAIN);
    free(post_html);
    return NULL;
  }

  // insert header info
  main_html = replace(main_html, "{STYLE}", CSS_POST);
  main_html = replace(main_html, "{SCRIPT}", "");

  // is client logged in?
  main_html = fill_nav_login(main_html, token);

  // insert post html into main
  main_html = replace(main_html, "{PAGE_BODY}", post_html);
  free(post_html);

  return main_html;
}


char* fill_community_info(char* template, char* community_name)
{
  // prepare query string
  char* query_fmt = QUERY_COMMUNITY_BY_NAME;
  char query[strlen(query_fmt) + strlen(community_name) + 1];
  sprintf(query, query_fmt, community_name);

  list** result = query_database_ls(query);

  // make sure we got exactly ONE entry
  if (result[0] == NULL)
  {
    fprintf(stderr, "community '%s' not found\n", community_name);
    free(template);
    free(result);
    return NULL;
  }

  if (result[1] != NULL)
  {
    fprintf(stderr, "fill_community_info(): multiple communities with name '%s'\n", community_name);
    exit(EXIT_FAILURE);
  }
  list* community_info = result[0];
  free(result);

  // fill in community info
  template = replace(template, "{COMMUNITY_NAME}", list_get(community_info, SQL_FIELD_COMMUNITY_NAME)->cp);
  template = replace(template, "{COMMUNITY_ABOUT}", list_get(community_info, SQL_FIELD_COMMUNITY_ABOUT)->cp);

  list_delete(community_info);

  return template;
}


char* fill_community_posts(char* template, char* community_name)
{
  // prepare query string
  char* query_fmt = QUERY_POSTS_BY_COMMUNITY_NAME;
  char query[strlen(query_fmt) + strlen(community_name) + 1];
  sprintf(query, query_fmt, community_name);

  // grab posts belonging to this community
  list** posts = query_database_ls(query);

  // load post stub template
  char* post_stub;
  if ((post_stub = load_file(HTML_COMMUNITY_POST_STUB)) == NULL)
  {
    // failed to load comment stub
    fprintf(stderr, "fill_community_posts(): failed to load file %s\n", HTML_COMMUNITY_POST_STUB);
    for (int i = 0; posts[i] != NULL; i++)
    {
      list_delete(posts[i]);
    }
    free(posts); 
    free(template);
    return NULL;
  }

  int post_stub_len = strlen(post_stub);

  // iterate over each post
  for (int i = 0; posts[i] != NULL; i++)
  {
    // copy stub string instead of read it from disk every loop
    char* post_stub_copy = malloc((post_stub_len + 1) * sizeof(char));
    memcpy(post_stub_copy, post_stub, post_stub_len + 1);

    // current post
    list* p = posts[i];
    post_stub_copy = replace(post_stub_copy, "{USER_NAME}", list_get(p, SQL_FIELD_POST_AUTHOR_NAME)->cp);
    post_stub_copy = replace(post_stub_copy, "{USER_NAME}", list_get(p, SQL_FIELD_POST_AUTHOR_NAME)->cp);
    post_stub_copy = replace(post_stub_copy, "{POST_ID}", list_get(p, SQL_FIELD_POST_ID)->cp);
    post_stub_copy = replace(post_stub_copy, "{POST_TITLE}", list_get(p, SQL_FIELD_POST_TITLE)->cp);
    post_stub_copy = replace(post_stub_copy, "{POST_BODY}", list_get(p, SQL_FIELD_POST_BODY)->cp);

    printf("%s\n", post_stub_copy);

    template = replace(template, "{POST_STUB}", post_stub_copy);

    free(post_stub_copy);
    list_delete(p);
  }

  free(posts);
  free(post_stub);

  // remove trailing template string
  return replace(template, "{POST_STUB}", "");
}


char* get_community(char* community_name, char* token)
{
  if (community_name == NULL || strlen(community_name) == 0)
  {
    return NULL;
  }

  // load community template
  char* community_html;
  if ((community_html = load_file(HTML_COMMUNITY)) == NULL)
  {
    fprintf(stderr, "get_community(): failed to load '%s'\n", HTML_COMMUNITY);
    return NULL;
  }

  // fill in community info
  if ((community_html = fill_community_info(community_html, community_name)) == NULL)
  {
    // community not found
    return NULL;
  }

  community_html = fill_community_posts(community_html, community_name);

  // load main html
  char* main_html;
  if ((main_html = load_file(HTML_MAIN)) == NULL)
  {
    fprintf(stderr, "get_community(): failed to load file '%s'\n", HTML_MAIN);
    free(community_html);
    return NULL;
  }

  // insert header info
  main_html = replace(main_html, "{STYLE}", CSS_COMMUNITY);
  main_html = replace(main_html, "{SCRIPT}", JS_COMMUNITY);

  // is client logged in?
  main_html = fill_nav_login(main_html, token);

  // insert community html into main
  main_html = replace(main_html, "{PAGE_BODY}", community_html);
  free(community_html);

  return main_html;
}

/*
char* get_home(char* token)
{
  // if user is not logged in, just return popular
  if (valid_token(token) == NULL)
  {
    return get_popular(token);
  }

  // TODO: load feed template

  // TODO: fill in post stubs using query that selects top ranking
  // posts from communities that client is subscribed to

  // load main template
  char* main_html;
  if ((main_html = load_file(HTML_MAIN)) == NULL)
  {
    fprintf(stderr, "get_home(): failed to load file '%s'\n", HTML_MAIN);
    return NULL;
  }

  // insert header info
  main_html = replace(main_html, "{STYLE}", CSS_FEED);
  main_html = replace(main_html, "{SCRIPT}", "");

  // we know client isn't logged in, so just plug in the 'Signup' link
  main_html = fill_nav_login(main_html, token);

  // TODO: insert feed html into main

  return main_html;
}


char* fill_popular_posts(char* template)
{
  // prepare query string
  char* query_fmt = 
}


char* get_popular(char* token)
{
  // load feed template
  char* feed_html;
  if ((feed_html = load_file(HTML_FEED)) == NULL)
  {
    fprintf(stderr, "get_popular(): failed to load file '%s'\n", HTML_FEED);
    return NULL;
  }

  // TODO: fill in post stubs using query that selects top ranking
  // posts from all communities
  feed_html = fill_popular_posts(feed_html)

  // top ranking posts from all communities

  // TODO: load and prep main template
  char* main_html;
  if ((main_html = load_file(HTML_MAIN)) == NULL)
  {
    fprintf(stderr, "get_popular(): failed to load file: '%s'\n", HTML_MAIN);
    return NULL;
  }

  // TODO: insert feed into main html

}
*/
