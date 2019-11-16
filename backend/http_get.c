
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "include/server.h"
#include "include/http_get.h"


char* http_get(char* request)
{
  char* uri, *content, *resp, *err;
  int resplen = 0;
  FILE* file;

  resp = calloc(1, BUFFERLEN);

  /* what are we getting? */
  uri = parse_uri(request);

  /* do we have it? */
  if ((content = load_file(uri)) == NULL)
  {
    free(uri);
    memcpy(resp, STAT404, sizeof(STAT404));
    return resp;
  }

  /* what type of file is it? */
  

  /* how big is it? */

  /* ship it! */
  char* ok = "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: 2\n\nYO";
  resp = calloc(1, strlen(ok));
  memcpy(resp, ok, strlen(ok));
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


char* load_file(char* uri)
{
  int filelen;
  char* content;
  FILE* fd;

  if ((fd = fopen(uri, "r")) == NULL)
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

