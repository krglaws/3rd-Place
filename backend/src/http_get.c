
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <kylestructs.h>

#include <common.h>
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
    content = get_user(req->uri + 4);
  }
  else if (strstr(req->uri, "./c/"))
  {
    content_type = TEXTHTML;
    content = get_community(req->uri + 4);
  }
  else if (strstr(req->uri, "./p/"))
  {
    content_type = TEXTHTML;
    content = get_post(req->uri + 4);
  }

  /* do we have it? */
  if (content_type == NULL || content == NULL)
  {
    if (content != NULL)
    {
      free(content);
    }
    return send_404();
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


datacont* load_file(char* path)
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

  char content[filelen];

  for (int i = 0; i < filelen; i++)
  {
    content[i] = fgetc(fd);
  }
 
  fclose(fd);

  return datacont_new(content, CHARP, filelen);
}


struct response* get_user(char* uname)
{
  int len = strlen(uname);

  if (len == 0)
  {
    return send_err(STAT404);
  }

  char* query_fmt = "SELECT * FROM posts WHERE authid = \
		 (SELECT uuid FROM users WHERE uname = %s);";
  char query[strlen(query) + len + 1];
  sprintf(query, query_fmt, uname);
  list** result = query_database(query);


}

/*
list** query_database_ls(char* query)
{
  char*** result = query_database(query);
  list** result_ls = NULL;

  int i = 0;
  char** row;
  while ((row = *(result + (i)) != NULL)
  {
  */
    /* create new list for this row */
/*
    result_ls = realloc(result_ls, (i + 1) * sizeof(list *));
    *(result_ls + i) = list_new();

    int j = 0;
    char* cell;
    while ((cell = *(row + j)) != NULL)
    {
      list_add(*(result_ls + i), datacont_new(cell, CHARP, strlen(cell)));
      free(cell);
    }
    free(row);
  }
  free(result);

  return result_ls;
}
*/
