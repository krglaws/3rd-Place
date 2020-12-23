#ifndef _GET_FEED_H_
#define _GET_FEED_H_

#include <auth_manager.h>

char* get_feed(const char* home_or_popular, const struct auth_token* client_info);

#endif