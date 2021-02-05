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

struct response* get_new_post(const char* community_id, const struct auth_token* client_info);

#endif
