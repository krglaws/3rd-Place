#ifndef _GET_USER_H_
#define _GET_USER_H_

#include <kylestructs.h>
#include <auth_manager.h>

char* fill_user_info(char* template, const char* user_name);

char* fill_user_comment_template(char* template, const ks_list* comment_info, const struct auth_token* client_info);

char* fill_user_comments(char* template, const char* user_name, const struct auth_token* client_info);

char* fill_user_post_template(char* template, const ks_list* post_info, const struct auth_token* client_info);

char* fill_user_posts(char* template, char* user_name, const struct auth_token* client_info);

char* get_user(char* user_name, const struct auth_token* client_info);

#endif
