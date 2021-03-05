#ifndef GET_POST_H
#define GET_POST_H

#include <kylestructs.h>
#include <auth_manager.h>

struct response* get_post(const char* post_id, const struct auth_token* client_info);

#endif
