#ifndef _GET_COMMUNITY_H_
#define _GET_COMMUNITY_H_

#include <kylestructs.h>
#include <auth_manager.h>

char* fill_community_info(char* template, const char* community_name);

char* fill_community_post_template(char* template, const ks_list* post_info, const struct auth_token* client_info);

char* fill_community_posts(char* template, const char* community_name, const struct auth_token* client_info);

char* get_community(const char* community_name, const struct auth_token* client_info);

#endif
