
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <kylestructs.h>

#include "include/common.h"
#include "include/http_get.h"


struct response* http_get(struct request* req)
{
  if (req == NULL) return NULL;

  char *uri, *conttype;
  datacont* content;
  int contlen;

  /* what are we getting? */
  datacont* line1 = list_get(req->header, 0);
  uri = parse_uri(line1->cp);

  if (strstr(uri, ".html"))
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

