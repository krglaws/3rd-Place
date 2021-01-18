#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <kylestructs.h>

#include <common.h>
#include <log_manager.h>
#include <auth_manager.h>
#include <senderr.h>


static const char* contlenhdr = "Connection: close\r\nContent-Length: 3\r\n";

struct response* senderr(enum http_errno eno)
{
  struct response* resp = calloc(1, sizeof(struct response));

  resp->header = ks_list_new();
  resp->content_length = 3;
  resp->content = malloc(3 * sizeof(char));

  switch (eno)
  {
  case ERR_BAD_REQ: 
    ks_list_add(resp->header, ks_datacont_new(STAT400, KS_CHARP, strlen(STAT400)));
    memcpy(resp->content, "400", 3);
    break;
  case ERR_NOT_FOUND:
    ks_list_add(resp->header, ks_datacont_new(STAT404, KS_CHARP, strlen(STAT404)));
    memcpy(resp->content, "404", 3);
    break;
  case ERR_MSG_TOO_BIG:
    ks_list_add(resp->header, ks_datacont_new(STAT404, KS_CHARP, strlen(STAT413)));
    memcpy(resp->content, "413", 3);
    break;
  case ERR_URI_TOO_LONG:
    ks_list_add(resp->header, ks_datacont_new(STAT404, KS_CHARP, strlen(STAT414)));
    memcpy(resp->content, "414", 3);
    break;
  case ERR_INTERNAL:
    ks_list_add(resp->header, ks_datacont_new(STAT500, KS_CHARP, strlen(STAT500)));
    memcpy(resp->content, "500", 3);
    break;
  case ERR_NOT_IMPL:
    ks_list_add(resp->header, ks_datacont_new(STAT501, KS_CHARP, strlen(STAT501)));
    memcpy(resp->content, "501", 3);
    break;
  case ERR_HTTP_VERS_NOT_SUPP:
    ks_list_add(resp->header, ks_datacont_new(STAT505, KS_CHARP, strlen(STAT505)));
    memcpy(resp->content, "505", 3);
    break;
  default:
    log_crit("senderr(): unknown error response number");
  }

  ks_list_add(resp->header, ks_datacont_new(contlenhdr, KS_CHARP, strlen(contlenhdr)));

  return resp;
}


static const char* redir_tmp = "Location: %s\r\n";
static const char* contlen0 = "Content-Length: 0\r\n";

struct response* redirect(const char* uri)
{
  if (uri == NULL)
  {
    log_crit("redirect(): null redirect uri");
  }

  char redir_str[64];
  sprintf(redir_str, redir_tmp, uri);

  // create response object
  struct response* resp = calloc(1, sizeof(struct response));
  resp->header = ks_list_new();
  ks_list_add(resp->header, ks_datacont_new(STAT302, KS_CHARP, strlen(STAT302)));
  ks_list_add(resp->header, ks_datacont_new(redir_str, KS_CHARP, strlen(redir_str)));
  ks_list_add(resp->header, ks_datacont_new(contlen0, KS_CHARP, strlen(contlen0)));

  return resp;
}
