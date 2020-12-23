#include <stdlib.h>
#include <kylestructs.h>

#include <common.h>
#include <auth_manager.h>
#include <senderr.h>


static const char* contlenhdr = "Connection: close\nContent-Length: 3\n";

struct response* senderr(enum http_errno eno)
{
  struct response* resp = calloc(1, sizeof(struct response));

  resp->header = list_new();
  resp->content_length = 3;
  resp->content = malloc(3 * sizeof(char));

  switch (eno)
  {
  case ERR_BAD_REQ: 
    list_add(resp->header, datacont_new(STAT400, CHARP, strlen(STAT400)));
    memcpy(resp->content, "400", 3);
    break;
  case ERR_NOT_FOUND:
    list_add(resp->header, datacont_new(STAT404, CHARP, strlen(STAT404)));
    memcpy(resp->content, "404", 3);
    break;
  case ERR_MSG_TOO_BIG:
    list_add(resp->header, datacont_new(STAT404, CHARP, strlen(STAT413)));
    memcpy(resp->content, "413", 3);
    break;
  case ERR_URI_TOO_LONG:
    list_add(resp->header, datacont_new(STAT404, CHARP, strlen(STAT414)));
    memcpy(resp->content, "414", 3);
    break;
  case ERR_INTERNAL:
    list_add(resp->header, datacont_new(STAT500, CHARP, strlen(STAT500)));
    memcpy(resp->content, "500", 3);
    break;
  case ERR_NOT_IMPL:
    list_add(resp->header, datacont_new(STAT501, CHARP, strlen(STAT501)));
    memcpy(resp->content, "501", 3);
    break;
  case ERR_HTTP_VERS_NOT_SUPP:
    list_add(resp->header, datacont_new(STAT505, CHARP, strlen(STAT505)));
    memcpy(resp->content, "505", 3);
    break;
  }

  list_add(resp->header, datacont_new(contlenhdr, CHARP, strlen(contlenhdr)));

  return resp;
}


static const char* redir_str = HTTPVERSION "Location: /login\n";

struct response* login_redirect()
{
  // create response object
  struct response* resp = calloc(1, sizeof(struct response));
  resp->header = list_new();
  list_add(resp->header, datacont_new(redir_str, CHARP, strlen(redir_str)));

  return resp;
}
