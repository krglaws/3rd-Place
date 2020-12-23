#ifndef _GET_POST_H_
#define _GET_POST_H_

#include <kylestructs.h>
#include <auth_manager.h>

char* fill_post_info(char* template, const char* post_id, const struct auth_token* client_info);

char* fill_post_comment_template(char* template, const list* comment_info, const struct auth_token* client_info);

char* fill_post_comments(char* template, const char* post_id, const struct auth_token* client_info);

char* get_post(const char* post_id, const struct auth_token* client_info);

#endif