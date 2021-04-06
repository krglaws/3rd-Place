#define _XOPEN_SOURCE
#include <unistd.h>
#include <crypt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <kylestructs.h>

#include <string_map.h>
#include <log_manager.h>
#include <sql_manager.h>
#include <auth_manager.h>


static char rand_byte();
static void rand_salt(char* salt_buf);
static void rand_token(char* token_buf);
static const char* new_token(const char* user_name);
static const char* get_token(const char* user_name);
static void terminate_auth_manager();

/* ks_list of login tokens */
static ks_list* token_list;


// file for reading from /dev/urandom for secure
// number generation.
static FILE* urandom = NULL;


// These are the 64 chars that can be used in the
// 'salt' string for the crypt function. Base64 
// might be kind of a misnomer since the order is wrong,
// and there's supposed to be a '+' instead of '.', but
// that doesnt really matter here.

// 64 chars used by rand_byte()
static const char base64[64] = "./0123456789ABCD" \
                               "EFGHIJKLMNOPQRST" \
                               "UVWXYZabcdefghij" \
                               "klmnopqrstuvwxyz";


void init_auth_manager()
{
  if ((urandom = fopen("/dev/urandom", "r")) == NULL)
  {
    log_info("Failed to open /dev/urandom: %s\nUsing rand() instead", strerror(errno));
    srand(time(NULL));
  }

  token_list = ks_list_new();

  atexit(&terminate_auth_manager);
}


static void terminate_auth_manager()
{
  log_info("Terminating Auth Manager...");

  if (urandom != NULL)
  {
    fclose(urandom);
  }
  // iterate through token list, delete each auth token struct
  // (auth token deletion has to be done separately since
  // ks_datacont_delete doesnt call free() on void pointer type.
  ks_datacont* curr;
  ks_iterator* iter = ks_iterator_new(token_list, KS_LIST);
  while ((curr = (ks_datacont*)ks_iterator_next(iter)) != NULL)
  {
    free(curr->vp);
    curr->vp = NULL;
  }
  ks_iterator_delete(iter);
  ks_list_delete(token_list);
}


static char rand_byte()
{
  int index = (int) (urandom ? fgetc(urandom) : rand()) % 64;
  return base64[index];
}


static void rand_salt(char* saltbuf)
{
  saltbuf[0] = rand_byte();
  saltbuf[1] = rand_byte();
  saltbuf[2] = '\0';

  return;
}


static void rand_token(char* token_buf)
{
  for (int i = 0; i < TOKENLEN; i++)
  {
    token_buf[i] = rand_byte();
  }
}


const char* login_user(const char* user_name, const char* password)
{
  if (user_name == NULL || password == NULL)
  {
    return NULL;
  }

  ks_hashmap* user_info;
  if ((user_info = query_user_by_name(user_name)) == NULL)
  {
    // user does not exist
    return NULL;
  }

  // grab user hash from query
  const char* hash1 = get_map_value_str(user_info, FIELD_USER_PASSWORD_HASH);

  char salt[3];
  salt[0] = hash1[0];
  salt[1] = hash1[1];
  salt[2] = '\0';

  // hash submitted password
  char* hash2;
  if ((hash2 = crypt(password, salt)) == NULL)
  {
    return NULL;
  }

  // compare hashes
  if (strcmp(hash1, hash2) != 0)
  {
    ks_hashmap_delete(user_info);
    // invalid password
    return NULL;
  }
  ks_hashmap_delete(user_info);

  // user and password match, retrieve token if already exists,
  // else create new token and return
  const char* token;
  if ((token = get_token(user_name)) != NULL)
  {
    return token;
  }

  if ((token = new_token(user_name)) == NULL)
  {
    log_err("login_user(): failed to create new user token");
    return NULL;
  }

  return token;
}


const char* new_user(const char* user_name, const char* password, const char* about)
{
  if (user_name == NULL || password == NULL)
  {
    // this should never happen
    return NULL;
  }

  // check if user already exists
  ks_hashmap* user_info = query_user_by_name(user_name);

  if (user_info != NULL)
  {
    ks_hashmap_delete(user_info);
    return NULL;
  }

  // hash user password
  char salt[3];
  rand_salt(salt);
  char* pwhash = crypt(password, salt);

  // prepare insert query
  char* user_id;
  if ((user_id = sql_create_user(user_name, pwhash, about)) == NULL)
  {
    log_err("new_user(): failed to create new user");
    return NULL;
  }
  free(user_id);

  const char* token;
  if ((token = new_token(user_name)) == NULL)
  {
    log_err("new_user(): failed to create new user token");
    return NULL;
  }

  return token;
}


int update_password(const char* user_id, const char* password)
{
  // create salt and hash password
  char salt[3];
  rand_salt(salt);
  char* pwhash = crypt(password, salt);

  // store into database
  return sql_update_user_password_hash(user_id, pwhash);
}


static const char* new_token(const char* user_name)
{
  // pull user info from database
  ks_hashmap* user_info = query_user_by_name(user_name);
  const char* user_id = get_map_value_str(user_info, FIELD_USER_ID);

  // copy info into auth token
  struct auth_token* new_token = calloc(1, sizeof(struct auth_token));
  memcpy(new_token->user_id, user_id, strlen(user_id));
  memcpy(new_token->user_name, user_name, strlen(user_name));
  new_token->last_active = time(NULL);
  ks_hashmap_delete(user_info);

  // generate random token
  do
  {
    rand_token(new_token->token);
  } while (valid_token(new_token->token) != NULL);

  // append token entry to ks_list
  ks_list_add(token_list, ks_datacont_new(new_token, KS_VOIDP, 1));

  // return token string
  return new_token->token;
}


static const char* get_token(const char* user_name)
{
  if (user_name == NULL)
  {
    return NULL;
  }

  const ks_datacont* curr;
  ks_iterator* iter = ks_iterator_new(token_list, KS_CHARP);
  while ((curr = ks_iterator_next(iter)) != NULL)
  {
    struct auth_token* at = curr->vp;
    if (strcmp(at->user_name, user_name) == 0)
    {
      ks_iterator_delete(iter);
      return at->token;
    }
  }
  ks_iterator_delete(iter);

  return NULL;
}


// called by http_post() for logout
void remove_token(const char* token)
{
  if (token == NULL)
  {
    // should be unreachable
    log_crit("remove_token(): null token argument");
  }

  int index = 0;
  ks_datacont* curr;
  ks_iterator* iter = ks_iterator_new(token_list, KS_LIST);
  while ((curr = (ks_datacont*) ks_iterator_next(iter)) != NULL)
  {
    struct auth_token* at = curr->vp;
    if (strcmp(token, at->token) == 0)
    {
      free(at);
      curr->vp = NULL;
      ks_iterator_delete(iter);
      ks_list_remove_at(token_list, index);
      return;
    }
    index++;
  }
  ks_iterator_delete(iter);

  log_err("remove_token(): failed to find matching token string");
}


const struct auth_token* valid_token(const char* token)
{
  if (token == NULL)
  {
    return NULL;
  }

  // check all tokens for expiration
  check_tokens();

  const ks_datacont* curr;
  ks_iterator* iter = ks_iterator_new(token_list, KS_LIST);
  while ((curr = ks_iterator_next(iter)) != NULL)
  {
    struct auth_token* at = curr->vp;
    if (strcmp(token, at->token) == 0)
    {
      // reset last active time
      at->last_active = time(NULL);
      ks_iterator_delete(iter);
      return at;
    }
  }
  ks_iterator_delete(iter);

  // token not found
  return NULL;
}


void check_tokens()
{
  time_t currtime = time(NULL);

  // compile list of expired tokens
  int index = 0;
  ks_datacont* curr;
  ks_list* dellist = ks_list_new();
  ks_iterator* iter = ks_iterator_new(token_list, KS_LIST);
  while ((curr = (ks_datacont*) ks_iterator_next(iter)) != NULL)
  {
    struct auth_token* at = curr->vp;
    int elapsed = (currtime - at->last_active);
    if (elapsed > TOKENLIFESPAN)
    {
      free(at);
      curr->vp = NULL;
      ks_list_add(dellist, ks_datacont_new(&index, KS_INT, 1));
    }
    index++;
  }
  ks_iterator_delete(iter);

  // delete all expired tokens
  iter = ks_iterator_new(dellist, KS_LIST);
  while ((curr = (ks_datacont*) ks_iterator_next(iter)) != NULL)
  {
    ks_list_remove_at(token_list, curr->i);
  }
  ks_iterator_delete(iter);
  ks_list_delete(dellist);
}
