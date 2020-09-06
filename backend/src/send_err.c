
#include <stdlib.h>
#include <kylestructs.h>

#include <common.h>
#include <send_err.h>

struct response* send_err(char* status)
{
  char* header = "Connection: close\nContent-Length: 0\n";
  struct response* resp = calloc(1, sizeof(struct response));
  resp->header = list_new();

  list_add(resp->header, datacont_new(status, CHARP, strlen(status)));
  list_add(resp->header, datacont_new(header, CHARP, strlen(header)));

  return resp;
}


