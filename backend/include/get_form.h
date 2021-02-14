#ifndef GET_NEW_H
#define GET_NEW_H

enum login_error {
  LOGINERR_NONE,
  LOGINERR_BAD_LOGIN,
  LOGINERR_EMPTY,
  LOGINERR_UNAME_TAKEN
};

#define BAD_LOGIN_MSG "<p>Bad Login</p>"
#define UNAME_TAKEN_MSG "<p>Username Already Exists</p>"
#define EMPTY_INPUT_MSG "<p>Empty username or password</p>"

struct response* get_login(const struct auth_token* client_info, enum login_error err);

struct response* get_edit_user(const struct auth_token* client_info);

struct response* get_new_post(const char* community_id, const struct auth_token* client_info);

struct response* get_edit_post(const char* post_id, const struct auth_token* client_info);

struct response* get_new_comment(const char* post_id, const struct auth_token* client_info);

struct response* get_edit_comment(const char* comment_id, const struct auth_token* client_info);

struct response* get_new_community(const struct auth_token* client_info);

struct response* get_edit_community(const char* community_id, const struct auth_token* client_info);

#endif