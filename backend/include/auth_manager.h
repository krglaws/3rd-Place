#ifndef _AUTH_MANAGER_H_
#define _AUTH_MANAGER_H_

#include <kylestructs.h>

/* token struct definition */
#define UNAMELEN (16)
#define TOKENLEN (32)
#define TOKENLIFESPAN (7)


/* just a linked list to contain the
    auth tokens */
struct token_entry
{
  struct token_entry* next;
  struct auth_token* token;
  int days;
};


/* struct containing a user authorization token */
struct auth_token
{
  char token[TOKENLEN + 1];
  char user_id[32];
  char user_name[UNAMELEN + 2];
};


void init_auth_manager();

void terminate_auth_manager();

static char rand_byte();

static void random_salt(char* salt_buf);

static ks_list* get_user_info(const char* uname);

const char* new_user(const char* uname, const char* passwd);

static void random_token(char* token_buf);

static const char* new_token(const char* uname);

static const char* get_token(const char* uname);

void remove_token(const char* token);

const struct auth_token* valid_token(const char* token);

const char* login_user(const char* uname, const char* passwd);

void delete_token_entry(struct token_entry* t);

#endif
