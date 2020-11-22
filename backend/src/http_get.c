
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <kylestructs.h>

#include <common.h>
#include <util.h>
#include <send_err.h>
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
  if (strstr(req->uri, ".css"))
  {
    content_type = TEXTCSS;
    content = load_file(req->uri);
  }
  else if (strstr(req->uri, ".js"))
  {
    content_type = APPJS;
    content = load_file(req->uri);
  }
  else if (strstr(req->uri, ".ico"))
  {
    content_type = IMGICO;
    content = load_file(req->uri);
  }
  else if (strstr(req->uri, "./u/"))
  {
    content_type = TEXTHTML;
    content = get_user(req->uri + 4, req->token);
  }
/*  else if (strstr(req->uri, "./c/"))
  {
    content_type = TEXTHTML;
    content = get_community(req->uri + 4, req->token);
  }
  else if (strstr(req->uri, "./p/"))
  {
    content_type = TEXTHTML;
    content = get_post(req->uri + 4, req->token);
  }
*/
  /* do we have it? */
  if (content_type == NULL || content == NULL)
  {
    if (content != NULL)
    {
      free(content);
    }
    return send_err(STAT404);
  }

  /* prepare response object */
  struct response* resp = calloc(1, sizeof(struct response));
  resp->header = list_new();
  list_add(resp->header, datacont_new(STAT200, CHARP, strlen(STAT200)));
  list_add(resp->header, datacont_new(content_type, CHARP, strlen(content_type)));
 
  char contline[80];
  int len = sprintf(contline, "Content-Length: %ld\n", strlen(content));

  list_add(resp->header, datacont_new(contline, CHARP, len));
  list_add(resp->header, datacont_new("Set-Cookie: test-cookie=abcde\n", CHARP, 30));
  resp->content = content;

  return resp;
}


list* get_user_info(char* uname)
{
  char* query_fmt = "SELECT * FROM users WHERE uname = '%s';";
  char query[strlen(uname) + strlen(query_fmt) + 1];

  sprintf(query, query_fmt, uname);

  list** result = query_database_ls(query);

  if (result[0] == NULL)
  {
    fprintf(stderr, "user %s not found\n", uname);
    free(result);
    return NULL;
  }

  if (result[1] != NULL)
  {
    fprintf(stderr, "Fatal error: multiple users with name %s\n", uname);
    exit(EXIT_FAILURE);
  }

  list* user_info = result[0];
  free(result);

  return user_info;
}


char* fill_in_posts(char* template, char* uname)
{
  // prepare query string
  char* query_fmt = "SELECT * FROM posts WHERE author = '%s';";
  char query[strlen(query_fmt) + strlen(uname) + 1];
  sprintf(query, query_fmt, uname);

  // grab user post list from database
  list** posts = query_database_ls(query);

  // iterate through each post
  for (int i = 0; posts[i] != NULL; i++)
  {
    // current post (list of fields)
    list* p = posts[i];

    // load post_stub template and do replacements
    char* post_stub = load_file("templates/user/post_stub.html");
    post_stub = replace(post_stub, "{POST_ID}", list_get(p, SF_POST_ID)->cp);
    post_stub = replace(post_stub, "{POST_TITLE}", list_get(p, SF_POST_TITLE)->cp);
    post_stub = replace(post_stub, "{COMMUNITY_NAME}", list_get(p, SF_POST_COMMUNITY_NAME)->cp);

    // limit post body to 64 chars in stub
    char body[65];
    int end = sprintf(body, "%61s", list_get(p, SF_POST_BODY)->cp);
    memcpy(body + end, "...", 3);
    body[end+3] = '\0';
    post_stub = replace(post_stub, "{POST_BODY}", body);

    // copy stub into main template
    template = replace(template, "{POST_STUB}", post_stub);

    free(post_stub);
    list_delete(p);
  }

  free(posts);

  // remove trailing template string
  return replace(template, "{POST_STUB}", "");
}


char* fill_in_comments(char* template, char* uname)
{
  // prepare query string
  char* query_fmt = "SELECT * FROM comments WHERE author = '%s';";
  char query[strlen(query_fmt) + strlen(uname) + 1];
  sprintf(query, query_fmt, uname);

  // grab user comment list from database
  list** comments = query_database_ls(query);

  // iterate over each comment
  for (int i = 0; comments[i] != NULL; i++)
  {
    // current comment (list of fields)
    list* c = comments[i];

    // load comment_stub template and do replacements
    char* comment_stub = load_file("templates/user/comment_stub.html");
    comment_stub = replace(comment_stub, "{POST_ID}", list_get(c, SF_COMMENT_POST_ID)->cp);
    comment_stub = replace(comment_stub, "{COMMENT_ID}", list_get(c, SF_COMMENT_ID)->cp);

    // limit comment body to 64 chars in stub
    char body[65];
    int end = sprintf(body, "%61s", list_get(c, SF_COMMENT_BODY)->cp);
    memcpy(body + end, "...", 3);
    body[end+3] = '\0';
    comment_stub = replace(comment_stub, "{COMMENT_BODY}", body);
    comment_stub = replace(comment_stub, "{POST_TITLE}", list_get(c, SF_COMMENT_POST_TITLE)->cp);
    comment_stub = replace(comment_stub, "{COMMUNITY_NAME}", list_get(c, SF_COMMENT_COMMUNITY_NAME)->cp);

    // copy stub into main template
    template = replace(template, "{COMMENT_STUB}", comment_stub);

    free(comment_stub);
    list_delete(c);
  }

  free(comments);

  // remove trailing template string
  return replace(template, "{COMMENT_STUB}", "");
}


char* get_user(char* uname, char* token)
{
  if (uname == NULL || strlen(uname) == 0)
  {
    return NULL;
  }

  // pull user info from database
  list* user_info = get_user_info(uname);
  if (user_info == NULL)
  {
    return NULL;
  }

  // load main template
  char* main_html = load_file("templates/main/main.html");

  // insert style link
  main_html = replace(main_html, "{STYLE}", "<link rel=\"stylesheet\" href=\"/templates/user/user.css\">");

  // insert js link
  main_html = replace(main_html, "{SCRIPT}", "<script src=\"/templates/user/user.js\"></script>");

  // check if client is logged in
  char temp_str[128];
  const char* client_uname = valid_token(token);
  if (client_uname != NULL)
  {
    sprintf(temp_str, "<a href=\"/u/%s\">%s</a>", client_uname, client_uname);
    main_html = replace(main_html, "{UNAME}", temp_str);
  }
  else
  {
    main_html = replace(main_html, "{UNAME}", 
               "<a href=\"/signup\">signup</a>");
  }

  // load user template
  char* user_html = load_file("templates/user/user.html");

  // fill in user info
  user_html = replace(user_html, "{USER_NAME}", uname);
  user_html = replace(user_html, "{USER_POINTS}", list_get(user_info, SF_USER_POINTS)->cp);
  user_html = replace(user_html, "{USER_POSTS}", list_get(user_info, SF_USER_POSTS)->cp);
  user_html = replace(user_html, "{USER_COMMENTS}", list_get(user_info, SF_USER_COMMENTS)->cp);
  user_html = replace(user_html, "{USER_BDAY}", list_get(user_info, SF_USER_BDAY)->cp);
  user_html = replace(user_html, "{USER_ABOUT}", list_get(user_info, SF_USER_ABOUT)->cp);

  // fill in posts
  user_html = fill_in_posts(user_html, uname);

  // fill in comments
  user_html = fill_in_comments(user_html, uname);

  // insert user_data into main html
  main_html = replace(main_html, "{PAGE_BODY}", user_html);

  free(user_html);
  list_delete(user_info);

  return main_html;
}


char* replace(char* template, char* this, char* withthat)
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


/*
char* get_feed(datacont* community, int offset)
{
  int len = strlen(community);

  if (len = 0 || len > 32)
    return NULL;

  char[] post_query_template = "SELECT * FROM posts WHERE commid = \
		      (SELECT uuid FROM communities WHERE name = '%s')\
		      order by uuid desc limit %d,10;"

  char post_query[sizeof(post_query_template) + 32 + 12] = {0};
  sprintf(post_query, post_query_template, community->cp, offset);

  list** result = query_database(query);

  for (int i = 0; result[i]; i++)
  {
    list* row = result[i];
    int len = list_length(row);
    for (int j = 0; j < len; j++)
    {
      datacont* cell = list_get(row, j);
      
    }
    list_delete(result[i]);
  }

  datacont* template = load_file("./templates/post.hml");
  list* feedlist = list_new();
}


struct response* get_community(char* uri)
{
  if (uri == NULL)
  {
    fprintf(stderr, "get_community(): NULL URI");
    exit(EXIT_FAILURE);
  }

  char* cname = strstr(uri, "./c/") + 4;

}
*/

