#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <kylestructs.h>

#include <server.h>
#include <log_manager.h>
#include <auth_manager.h>
#include <response.h>


static const char* cont_len_tmplt = "Content-Length: %d\r\n";
static const char* redir_tmplt = "Location: %s\r\n";
static const char* cont_len_0 = "Content-Length: 0\r\n";
static const char* conn_close = "Connection: close\r\n";


struct response* response_error(const char* errstr)
{
  if (errstr == NULL)
  {
    log_crit("response_error(): NULL error string");
  }

  // allocate object
  struct response* resp = calloc(1, sizeof(struct response));

  // add response body
  resp->content_length = strlen(errstr);
  resp->content = malloc(resp->content_length + 1);
  memcpy(resp->content, errstr, resp->content_length + 1);

  // print error response header
  char contlenline[64];
  int len = sprintf(contlenline, cont_len_tmplt, resp->content_length);

  // add response header
  resp->header = ks_list_new();
  ks_list_add(resp->header, ks_datacont_new(errstr, KS_CHARP, resp->content_length));
  ks_list_add(resp->header, ks_datacont_new(conn_close, KS_CHARP, strlen(conn_close)));
  ks_list_add(resp->header, ks_datacont_new(contlenline, KS_CHARP, len));

  return resp;
}


struct response* response_redirect(const char* uri)
{
  if (uri == NULL)
  {
    log_crit("response_redirect(): NULL redirect uri");
  }

  char redir_str[64];
  int len = sprintf(redir_str, redir_tmplt, uri);

  // create response object
  struct response* resp = calloc(1, sizeof(struct response));
  resp->header = ks_list_new();
  ks_list_add(resp->header, ks_datacont_new(STAT302, KS_CHARP, strlen(STAT302)));
  ks_list_add(resp->header, ks_datacont_new(redir_str, KS_CHARP, len));
  ks_list_add(resp->header, ks_datacont_new(cont_len_0, KS_CHARP, strlen(cont_len_0)));

  return resp;
}


struct response* response_ok(const char* content)
{
  if (content == NULL)
  {
    log_crit("package_ok_response(): NULL content string");
  }

  // create response object
  struct response* resp = calloc(1, sizeof(struct response));
  resp->content = (char*) content;
  resp->content_length = strlen(content);

  // write content-length header
  char contlenline[64];
  int len = sprintf(contlenline, cont_len_tmplt, resp->content_length);

  // assume this is HTML being sent
  const char* content_type = "Content-Type: text/html\r\n";

  // build header
  resp->header = ks_list_new();
  ks_list_add(resp->header, ks_datacont_new(STAT200, KS_CHARP, strlen(STAT200)));
  ks_list_add(resp->header, ks_datacont_new(content_type, KS_CHARP, strlen(content_type)));
  ks_list_add(resp->header, ks_datacont_new(contlenline, KS_CHARP, len));

  return resp;
}
