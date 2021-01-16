#ifndef _AUTH_MANAGER_H_
#define _AUTH_MANAGER_H_

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


/* Closes /dev/urandom, deletes token_list
 */
void terminate_auth_manager();


/* returns a random base-64 character
 */
static char rand_byte();


/* Generates a random 2-character salt
 */
static void rand_salt(char* salt_buf);


/* Generates new token string
 */
static void rand_token(char* token_buf);


/* Grabs user info from database
 */
static ks_hashmap* get_user_info(const char* uname);


/* Called by http_post when a user tries to login.
 */
const char* login_user(const char* uname, const char* passwd);


/* Called by http_post when user signs up
 */
const char* new_user(const char* uname, const char* passwd);


/* Creates a new token, either after login or
 * signup.
 */
static const char* new_token(const char* uname);


/* Attempts to retrieve an existing token when user logs
 * in. Useful if user already has a token in token_list
 */
static const char* get_token(const char* uname);


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
