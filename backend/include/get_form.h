#ifndef GET_NEW_H
#define GET_NEW_H

enum user_form_error {
  USER_FORM_ERR_NONE,

  // login error
  USER_FORM_ERR_BAD_LOGIN,

  // signup uname error
  USER_FORM_ERR_UNAME_TAKEN,
  USER_FORM_ERR_UNAME_TOO_SHORT,
  USER_FORM_ERR_UNAME_TOO_LONG,
  USER_FORM_ERR_UNAME_INV_CHAR,

  // signup password error
  USER_FORM_ERR_PASSWD_TOO_SHORT,
  USER_FORM_ERR_PASSWD_TOO_LONG,
  USER_FORM_ERR_PASSWD_INV_ENC,
  USER_FORM_ERR_PASSWD_UNMET,
  USER_FORM_ERR_PASSWD_MISMATCH,

  // user about error
  USER_FORM_ERR_ABOUT_TOO_SHORT,
  USER_FORM_ERR_ABOUT_TOO_LONG,
  USER_FORM_ERR_ABOUT_INV_ENC
};

struct response* get_login(const char* submitted_user_name, enum user_form_error err, const struct auth_token* client_info);

struct response* get_edit_user(const char* submitted_about, enum user_form_error err, const struct auth_token* client_info);


enum post_form_error {
  POST_FORM_ERR_NONE,
  POST_FORM_ERR_TITLE_TOO_SHORT,
  POST_FORM_ERR_TITLE_TOO_LONG,
  POST_FORM_ERR_TITLE_INV_ENC,
  POST_FORM_ERR_BODY_TOO_SHORT,
  POST_FORM_ERR_BODY_TOO_LONG,
  POST_FORM_ERR_BODY_INV_ENC
};

struct response* get_new_post(const char* submitted_title, const char* submitted_body, const char* community_id, enum post_form_error err, const struct auth_token* client_info);

struct response* get_edit_post(const char* submitted_body, const char* post_id, enum post_form_error err, const struct auth_token* client_info);


enum comment_form_error {
  COMMENT_FORM_ERR_NONE,
  COMMENT_FORM_ERR_TOO_SHORT,
  COMMENT_FORM_ERR_TOO_LONG,
  COMMENT_FORM_ERR_INV_ENC
};

struct response* get_new_comment(const char* submitted_body, const char* post_id, enum comment_form_error err, const struct auth_token* client_info);

struct response* get_edit_comment(const char* submitted_body, const char* comment_id, enum comment_form_error err, const struct auth_token* client_info);


enum community_form_error {
  COMMUNITY_FORM_ERR_NONE,
  COMMUNITY_FORM_ERR_NAME_TOO_SHORT,
  COMMUNITY_FORM_ERR_NAME_TOO_LONG,
  COMMUNITY_FORM_ERR_NAME_INV_CHAR,
  COMMUNITY_FORM_ERR_ABOUT_TOO_SHORT,
  COMMUNITY_FORM_ERR_ABOUT_TOO_LONG,
  COMMUNITY_FORM_ERR_ABOUT_INV_ENC
};

struct response* get_new_community(const char* submitted_name, const char* submitted_about, enum community_form_error err, const struct auth_token* client_info);

struct response* get_edit_community(const char* submitted_about, const char* community_id, enum community_form_error err, const struct auth_token* client_info);

#endif
