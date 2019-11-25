
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <kylestructs.h>

#include "include/common.h"
#include "include/http_get.h"


struct response* http_get(struct request* req_str)
{
  /* what are we getting? */

  /* do we have it? */
 
  /* what type of file is it? */

  /* how big is it? */

  /* ship it! */

  free(req_str);
 
  char* h1 = "HTTP/1.1 200 OK\n";
  char* h2 = "Content-Type: text/html\n";
  char* h3 = "Content-Length: 2\n";

  struct response* resp = calloc(1, sizeof(struct response));
  resp->header = list_new();
  list_add(resp->header, datacont_new(h1, CHARP, strlen(h1)+1));
  list_add(resp->header, datacont_new(h2, CHARP, strlen(h2)+1));
  list_add(resp->header, datacont_new(h3, CHARP, strlen(h2)+1));

  resp->content = malloc(3 * sizeof(char));
  memcpy(resp->content, "OK", 3);

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

