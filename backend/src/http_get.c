
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <kylestructs.h>

#include <common.h>
#include <send_err.h>
#include <http_get.h>


static list* recent_posts;

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

struct response* http_get(struct request* req)
{
  if (req == NULL) return NULL;

  /* what are we getting? */
  char* content_type;
  char* content;
  if (strstr(req->uri, ".html"))
  {
    content_type = TEXTHTML;
    content = load_file(req->uri);
  }
  else if (strstr(req->uri, ".css"))
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
  list_add(resp->header, datacont_new(conttype, CHARP, strlen(conttype)));
 
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
  char* query_fmt = "SELECT * FROM users WHERE uname = %s;";
  char query[strlen(uname) + strlen(query_fmt) + 1];

  sprintf(query, query_fmt, uname);

  list** result = query_database_ls(query);

  if (result[0] == NULL)
  {
    fprintf(stderr, "user %s not found\n", uname);
    free(result);
    return NULL;
  }

  if (result[1] == NULL)
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
  char* query_fmt = "SELECT * FROM posts WHERE author = %s;";
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
  char* query_fmt = "SELECT * FROM comments WHERE author = %s;";
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


struct response* get_user(char* uname, char* token)
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

  char temp_str[128];

  // load main template
  char* index_tmp = load_file("templates/index.html.tmp");

  // check if client is logged in
  char* client_uname = valid_token(token);
  if (client_uname != NULL)
  {
    sprintf(temp_str, "<a href=\"/u/%s\">%s</a>", client_uname, client_uname);
    index_html = insert_html(index_html, "{UNAME}", temp_str);
  }
  else
  {
  index_html = insert_html(index_html, "{UNAME}", 
               "<p><a href=\"/signup\">signup</a>/<a href=\"/login\">login</a></p>");
  }

  // load user template
  char* user_html = load_file("templates/user.html.tmp");

  // fill in basic user info
  user_html = insert_html(user_html, "{UNAME}", uname);
  user_html = insert_html(user_html, "{NUM_POINTS}", list_get(*result, 2)->cp);
  user_html = insert_html(user_html, "{NUM_POSTS}", list_get(*result, 3)->cp);
  user_html = insert_html(user_html, "{NUM_COMMENTS}", list_get(*result, 4)->cp);
  user_html = insert_html(user_html, "{BDAYUTC}", list_get(*result, 5)->cp);

  // fill in posts

  // fill in comments

  // fill in about
}

