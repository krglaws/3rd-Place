#ifndef _GET_COMMUNITY_H_
#define _GET_COMMUNITY_H_

#include <kylestructs.h>
#include <auth_manager.h>

struct response* get_community(const char* community_name, const struct auth_token* client_info);

#endif
