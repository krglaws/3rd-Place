#ifndef GET_USER2_H
#define GET_USER2_H

#include <server.h>
#include <auth_manager.h>

struct response* get_user(const char* user_name, const struct auth_token* client_info);

#endif
