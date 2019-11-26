
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <kylestructs.h>

#include "include/common.h"
#include "include/http_get.h"


struct response* http_get(struct request* req)
{
  if (req == NULL) return NULL;

  char *uri, *conttype, *content;
  int contlen;

  /* what are we getting? */
  datacont* line1 = list_get(req->header, 0);
  uri = parse_uri(line1->cp);
  datacont_delete(line1);

  if (strstr(uri, ".html"))
    conttype = TEXTHTML;
  else if (strstr(uri, ".css"))
    conttype = TEXTCSS;
  else if (strstr(uri, ".js"))
    conttype = APPJS;
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
  /* how big is it? */
  contlen = strlen(content); 
  
  /* ship it! */
  list_delete(req->header);
  if (req->content) free(req->content);
  free(req);
  
  struct response* resp = calloc(1, sizeof(struct response));
  resp->header = list_new();
  list_add(resp->header, datacont_new(STAT200, CHARP, strlen(STAT200)+1));
  list_add(resp->header, datacont_new(conttype, CHARP, strlen(conttype)+1));
 
  char contline[80];
  sprintf(contline, "Content-Length: %d\n", contlen);
  list_add(resp->header, datacont_new(contline, CHARP, strlen(contline)+1));

  resp->content = content; 

  return resp;
}


struct response* send_404()
{
  struct response* resp = calloc(1, sizeof(struct response));
  resp->header = list_new();
  list_add(resp->header, datacont_new(STAT404, CHARP, strlen(STAT404)+1));
//  list_add(resp->header, datacont_new("Connection: close\n", CHARP, strlen("Connection: close\n")+1));
  list_add(resp->header, datacont_new("Content-Length: 0\n", CHARP, strlen("Content-Length: 0\n")+1));
  return resp;
}


char* parse_uri(char* request)
{
  char* uri, *getloc;
 
  uri = calloc(1, MAXURILEN);
  uri[0] = '.';

  getloc = strstr(request, "GET");
  sscanf(uri+1, "%*s %s", getloc);

  return uri;  
}


char* load_file(char* path)
{
  int filelen;
  char* content;
  FILE* fd;

  if ((fd = fopen(path, "r")) == NULL)
  {
    perror("http_get(): failed to open file");
    return NULL;
  }

  fseek(fd, 0, SEEK_END);
  filelen = ftell(fd);
  rewind(fd);

  content = calloc(1, filelen);
  for (int i = 0; i < filelen; i++)
    content[i] = fgetc(fd);
 
  fclose(fd);
 
  return content;
}

