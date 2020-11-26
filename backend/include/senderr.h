
#ifndef _SENDERR_H_
#define _SENDERR_H_

enum http_errno {
  BAD_REQUEST,
  NOT_FOUND,
  PAYLOAD_TOO_BIG,
  URI_TOO_LONG,
  INTERNAL_SERVER_ERROR,
  NOT_IMPLEMENTED,
  HTTP_VERSION_NOT_SUPPORTED
};

struct response* senderr(enum http_errno eno);

#endif

