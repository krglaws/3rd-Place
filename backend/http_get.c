
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "include/http_get.h"


char* http_get(char* request)
{
  char* ok = "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: 2\n\nYO";
  char* resp = calloc(1, strlen(ok));
  memcpy(resp, ok, strlen(ok));
  return resp;
}


