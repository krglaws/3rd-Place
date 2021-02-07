#ifndef _GET_FEED_H_
#define _GET_FEED_H_

#include <auth_manager.h>

enum feed_type
{
  HOME_FEED,
  POPULAR_FEED
};

struct response* get_feed(const enum feed_type ft, const struct auth_token* client_info);

#endif
