
#include <stdlib.h>
#include <kylestructs.h>

#include <common.h>
#include <senderr.h>


static char* contlenhdr = "Connection: close\nContent-Length: 3\n";

struct response* senderr(enum http_errno eno)
{
  struct response* resp = calloc(1, sizeof(struct response));

  resp->header = list_new();
  resp->content_length = 3;
  resp->content = malloc(3 * sizeof(char));

  switch (eno)
  {
  case BAD_REQUEST: 
    list_add(resp->header, datacont_new(STAT400, CHARP, strlen(STAT400)));
    memcpy(resp->content, "400", 3);
    break;
  case NOT_FOUND:
    list_add(resp->header, datacont_new(STAT404, CHARP, strlen(STAT404)));
    memcpy(resp->content, "404", 3);
    break;
  case PAYLOAD_TOO_BIG:
    list_add(resp->header, datacont_new(STAT404, CHARP, strlen(STAT413)));
    memcpy(resp->content, "413", 3);
    break;
  case URI_TOO_LONG:
    list_add(resp->header, datacont_new(STAT404, CHARP, strlen(STAT414)));
    memcpy(resp->content, "414", 3);
    break;
  case INTERNAL_SERVER_ERROR:
    list_add(resp->header, datacont_new(STAT500, CHARP, strlen(STAT500)));
    memcpy(resp->content, "500", 3);
    break;
  case NOT_IMPLEMENTED:
    list_add(resp->header, datacont_new(STAT501, CHARP, strlen(STAT501)));
    memcpy(resp->content, "501", 3);
    break;
  case HTTP_VERSION_NOT_SUPPORTED:
    list_add(resp->header, datacont_new(STAT505, CHARP, strlen(STAT505)));
    memcpy(resp->content, "505", 3);
    break;
  }

  list_add(resp->header, datacont_new(contlenhdr, CHARP, strlen(contlenhdr)));

  return resp;
}

