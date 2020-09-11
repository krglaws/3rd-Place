
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <kylestructs.h>

#include <common.h>
#include <send_err.h>
#include <templating.h>
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
/*
  else if (strstr(req->uri, "./c/"))
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


char* load_file(char* path)
{
  int filelen;
  FILE* fd;

  if ((fd = fopen(path, "r")) == NULL)
  {
    perror("http_get(): failed to open file");
    return NULL;
  }

  fseek(fd, 0, SEEK_END);
  filelen = ftell(fd);
  rewind(fd);

  char* content = malloc((filelen + 1) * sizeof(char));

  for (int i = 0; i < filelen; i++)
  {
    content[i] = fgetc(fd);
  }
  content[filelen] = '\0';
 
  fclose(fd);

  return content;
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


list** get_user_posts(char* uname)
{
  // figure out how to order this by date
  char* query_fmt = "SELECT * FROM posts WHERE author = '%s';";
  char query[strlen(query_fmt) + strlen(uname) + 1];

  sprintf(query, query_fmt, uname);

  list** result = query_database_ls(query);

  if (result[0] == NULL)
  {
    free(result);
    return NULL;
  }

  return result;
}


list** get_user_comments(char* uname)
{
  // figure out how to order this by date
  char* query_fmt = "SELECT * FROM comments WHERE author = '%s';";
  char query[strlen(query_fmt) + strlen(uname) + 1];

  sprintf(query, query_fmt, uname);

  list** result = query_database_ls(query);

  if (result[0] == NULL)
  {
    free(result);
    return NULL;
  }

  return result;
}


char* fill_in_posts(char* template, char* uname)
{
  char* query_fmt = "SELECT * FROM posts WHERE author = '%s';";
  char query[strlen(query_fmt) + strlen(uname) + 1];

  sprintf(query, query_fmt, uname);

  list** posts = query_database_ls(query);

  for (int i = 0; posts[i] != NULL; i++)
  {
    list* p = posts[i];

    char* post_stub = load_file("templates/post_stub.html.tmp");
    post_stub = replace_with(post_stub, "{TITLE}", list_get(p, 5)->cp);
    post_stub = replace_with(post_stub, "{COMMUNITY}", list_get(p, 2)->cp);

    char body[65];
    int end = sprintf(body, "%61s", list_get(p, 7)->cp);
    memcpy(body + end, "...", 3);
    body[end+3] = '\0';

    post_stub = replace_with(post_stub, "{BODY}", body);
    template = replace_with(template, "{POSTS}", post_stub);

    free(post_stub);
  }

  return replace_with(template, "{POSTS}", "");
}


char* fill_in_comments(char* template, char* uname)
{
  char* query_fmt = "SELECT * FROM comments WHERE author = '%s';";
  char query[strlen(query_fmt) + strlen(uname) + 1];

  sprintf(query, query_fmt, uname);

  list** comments = query_database_ls(query);

  for (int i = 0; comments[i] != NULL; i++)
  {
    char* comment_stub = load_file("templates/comment_stub");
    comment_stub = replace_with(comment_stub, "{POST}", list_get(comments[i], 4)->cp);

    char body[65];
    int end = sprintf(body, "%61s", list_get(comments[i], 7)->cp);
    memcpy(body + end, "...", 3);
    body[end+3] = '\0';

    comment_stub = replace_with(comment_stub, "{BODY}", body);
    template = replace_with(template, "{COMMENTS}", comment_stub);

    free(comment_stub);
  }

  return replace_with(template, "{COMMENTS}", "");
}


char* get_user(char* uname, char* token)
{
  if (uname == NULL || strlen(uname) == 0)
  {
    return NULL;
  }

  list* user_info = get_user_info(uname);
  if (user_info == NULL)
  {
    return NULL;
  }

  char temp_str[128]; // load main template
  char* index_html = load_file("templates/index.html.tmp");

  // check if client is logged in
  const char* client_uname = valid_token(token);
  if (client_uname != NULL)
  {
    sprintf(temp_str, "<a href=\"/u/%s\">%s</a>", client_uname, client_uname);
    index_html = replace_with(index_html, "{UNAME}", temp_str);
  }
  else
  {
  index_html = replace_with(index_html, "{UNAME}", 
               "<p><a href=\"/login\">login</a>/<a href=\"/signup\">signup</a></p>");
  }

  // load user template
  char* user_html = load_file("templates/user.html.tmp");

  // fill in user info
  user_html = replace_with(user_html, "{UNAME}", uname);
  user_html = replace_with(user_html, "{NUM_POINTS}", list_get(user_info, 3)->cp);
  user_html = replace_with(user_html, "{NUM_POSTS}", list_get(user_info, 4)->cp);
  user_html = replace_with(user_html, "{NUM_COMMENTS}", list_get(user_info, 5)->cp);
  user_html = replace_with(user_html, "{BDAYUTC}", list_get(user_info, 6)->cp);

  // fill in posts
  user_html = fill_in_posts(user_html, uname);

  // fill in comments
  user_html = fill_in_comments(user_html, uname);

  // fill in about
  user_html = replace_with(user_html, "{ABOUT}", list_get(user_info, 2)->cp);

  // insert user_data into main html
  index_html = replace_with(index_html, "{CONTENT}", user_html);

  free(user_html);
  list_delete(user_info);

  return index_html;
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

