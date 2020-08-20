
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
*/


struct response* get_community(char* uri)
{
  if (uri == NULL)
  {
    fprintf(stderr, "get_community(): NULL URI");
    exit(EXIT_FAILURE);
  }

  char* cname = strstr(uri, "./c/") + 4;

}


struct response* http_get(struct request* req)
{
  if (req == NULL) return NULL;

  char *uri, *conttype;
  datacont* content;
  int contlen;

  /* what are we getting? */
  datacont* line1 = list_get(req->header, 0);
  uri = parse_uri(line1->cp);

  /*if (strstr(uri, "./api/"))
  {
    free(uri);
    list_delete(req->header);
    if (req->content) free(req->content);
    free(req);
    return api_call(uri);
  }
  else if (strstr(uri, "./c/"))
  {
    free(uri);
    list_delete(req->header);
    if (req->content) free(req->content);
    free(req);
    return get_community(uri);
  }
  else if (strstr(uri, "./u/"))
  {
    free(uri);
    list_delete(req->header);
    if (req->content) free(req->content);
    free(req);
    return get_user(uri);
  }
  else if (strstr(uri, "./p/"))
  {
    free(uri);
    list_delete(req->header);
    if (req->content) free(req->content);
    free(req);
    return get_post(uri);
  }
  else */if (strstr(uri, ".html"))
    conttype = TEXTHTML;
  else if (strstr(uri, ".css"))
    conttype = TEXTCSS;
  else if (strstr(uri, ".js"))
    conttype = APPJS;
  else if (strstr(uri, ".ico"))
    conttype = IMGICO;
  else
  {
    free(uri);
    list_delete(req->header);
    if (req->content) free(req->content);
    free(req);
    return send_404();
  }

  /* do we have it? */
  if ((content = load_file(uri)) == NULL)
  {
    free(uri);
    list_delete(req->header);
    if (req->content) free(req->content);
    free(req);
    return send_404();
  }
  free(uri);

  /* ship it! */
  list_delete(req->header);
  datacont_delete(req->content); 
  free(req);

  struct response* resp = calloc(1, sizeof(struct response));
  resp->header = list_new();
  list_add(resp->header, datacont_new(STAT200, CHARP, strlen(STAT200)));
  list_add(resp->header, datacont_new(conttype, CHARP, strlen(conttype)));
 
  char contline[80];
  int len = sprintf(contline, "Content-Length: %ld\n", content->size);
  list_add(resp->header, datacont_new(contline, CHARP, len));

  resp->content = content;

  return resp;
}


struct response* send_404()
{
  struct response* resp = calloc(1, sizeof(struct response));
  resp->header = list_new();
  list_add(resp->header, datacont_new(STAT404, CHARP, strlen(STAT404)));
  list_add(resp->header, datacont_new("Connection: close\n", CHARP, strlen("Connection: close\n")));
  list_add(resp->header, datacont_new("Content-Length: 0\n", CHARP, strlen("Content-Length: 0\n")));
  return resp;
}


char* parse_uri(char* request)
{
  char* uri, *getloc;
 
  uri = calloc(1, MAXURILEN);
  uri[0] = '.';

  getloc = strstr(request, "GET");
  sscanf(getloc+4, "%s", uri+1);

  if (strcmp(uri, "./") == 0)
    memcpy(uri, "./index.html", strlen("./index.html")+1);

  return uri;
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
    content[i] = fgetc(fd);
 
  fclose(fd);

  return datacont_new(content, CHARP, filelen);
}

