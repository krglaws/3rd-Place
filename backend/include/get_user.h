#ifndef _GET_USER_H_
#define _GET_USER_H_

#include <common.h>
#include <kylestructs.h>

char* fill_user_info(char* template, const char* user_name);

char* fill_user_comment_template(char* template, const list* comment_info, const struct token_entry* client_info);

char* fill_user_comments(char* template, const char* user_name, const struct token_entry* client_info);

char* fill_user_post_template(char* template, const list* post_info, const struct token_entry* client_info);

char* fill_user_posts(char* template, char* user_name, const struct token_entry* client_info);

char* get_user(char* user_name, const struct token_entry* client_info);

#endif