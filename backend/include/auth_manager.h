#ifndef AUTH_MANAGER_H
#define AUTH_MANAGER_H

#include <time.h>
#include <kylestructs.h>

/* token struct definition */
#define UNAMELEN (16)
#define TOKENLEN (32)

/* seconds in a week */
#define TOKENLIFESPAN (86400 * 7)


/* struct containing a user authorization token */
struct auth_token
{
  time_t last_active;
  char token[TOKENLEN + 1];
  char user_id[32];
  char user_name[UNAMELEN + 2];
};


/* Opens /dev/urandom and initializes token_list
 */
void init_auth_manager();


/* Called by http_post when a user tries to login.
 */
const char* login_user(const char* uname, const char* password);


/* Called by http_post when user signs up
 */
const char* new_user(const char* uname, const char* password, const char* about);


/* Called by http_patch when user changes password
 */
int update_password(const char* user_id, const char* password);


/* Called by http_post when user logs out.
 */
void remove_token(const char* token);


/* Returns auth_token struct that contains token if present, else NULL.
 * This function is called whenever a client request has a logintoken
 * defined in its header.
 */
const struct auth_token* valid_token(const char* token);


/* Iterates through token_list looking for expired tokens.
 */
void check_tokens();

#endif
