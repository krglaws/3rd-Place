#ifndef GET_NEW_H
#define GET_NEW_H

enum login_error {
  LOGINERR_NONE,
  LOGINERR_BAD_LOGIN,
  SIGNUPERR_UNAME_TAKEN,
  SIGNUPERR_INVALID_UNAME,
  SIGNUPERR_INVALID_PASSWD
};

#define BAD_LOGIN_MSG "<p>Bad Login</p>"
#define UNAME_TAKEN_MSG "<p>Username Already Exists</p>"
#define INVALID_UNAME_MSG "<p>Invalid Username</p>"
#define INVALID_PASSWD_MSG "<p>Invalid Password</p>"

struct response* get_login(const struct auth_token* client_info, enum login_error err);

struct response* get_edit_user(const struct auth_token* client_info);

struct response* get_new_post(const char* community_id, const struct auth_token* client_info);

struct response* get_edit_post(const char* post_id, const struct auth_token* client_info);

struct response* get_new_comment(const char* post_id, const struct auth_token* client_info);

struct response* get_edit_comment(const char* comment_id, const struct auth_token* client_info);

struct response* get_new_community(const struct auth_token* client_info);

struct response* get_edit_community(const char* community_id, const struct auth_token* client_info);

#endif
