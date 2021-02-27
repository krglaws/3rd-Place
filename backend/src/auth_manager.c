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
#include <http_get.h>
#include <log_manager.h>
#include <sql_manager.h>
#include <auth_manager.h>


/* ks_list of login tokens */
static struct token_entry* head = NULL;

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
  log_info("Initializing Auth Mananger...");

  if ((urandom = fopen("/dev/urandom", "r")) == NULL)
  {
    log_info("Failed to open /dev/urandom: %s\nUsing rand() instead", strerror(errno));
    srand(time(NULL));
  }

  token_list = ks_list_new();
}


void terminate_auth_manager()
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
  while ((curr = (ks_datacont*)ks_iterator_get(iter)) != NULL)
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


const char* login_user(const char* uname, const char* passwd)
{
  if (uname == NULL || passwd == NULL)
  {
    return NULL;
  }

  ks_hashmap* user_info;
  if ((user_info = get_user_info(uname)) == NULL)
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

  // hash submitted passwd
  char* hash2;
  if ((hash2 = crypt(passwd, salt)) == NULL)
  {
    return NULL;
  }

  // compare hashes
  if (strcmp(hash1, hash2) != 0)
  {
    ks_hashmap_delete(user_info);
    // invalid passwd
    return NULL;
  }
  ks_hashmap_delete(user_info);

  // user and passwd match, retrieve token if already exists,
  // else create new token and return
  const char* token;
  if ((token = get_token(uname)) != NULL)
  {
    return token;
  }

  if ((token = new_token(uname)) == NULL)
  {
    log_err("login_user(): failed to create new user token");
    return NULL;
  }

  return token;
}


const char* new_user(const char* uname, const char* passwd, const char* about)
{
  if (uname == NULL || passwd == NULL)
  {
    // this should never happen
    return NULL;
  }

  // check if user already exists
  ks_hashmap* user_info = get_user_info(uname);

  if (user_info != NULL)
  {
    ks_hashmap_delete(user_info);
    return NULL;
  }

  // hash user passwd
  char salt[3];
  rand_salt(salt);
  char* pwhash = crypt(passwd, salt);

  // prepare insert query
  char* user_id;
  if ((user_id = sql_create_user(uname, pwhash, about)) == NULL)
  {
    log_err("new_user(): failed to create new user");
    return NULL;
  }
  free(user_id);

  const char* token;
  if ((token = new_token(uname)) == NULL)
  {
    log_err("new_user(): failed to create new user token");
    return NULL;
  }

  return token;
}



static const char* new_token(const char* uname)
{
  // pull user info from database
  ks_hashmap* user_info = get_user_info(uname);
  const char* user_id = get_map_value_str(user_info, FIELD_USER_ID);

  // copy info into auth token
  struct auth_token* new_token = calloc(1, sizeof(struct auth_token));
  memcpy(new_token->user_id, user_id, strlen(user_id));
  memcpy(new_token->user_name, uname, strlen(uname));
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


static const char* get_token(const char* uname)
{
  if (uname == NULL)
  {
    return NULL;
  }

  const ks_datacont* curr;
  ks_iterator* iter = ks_iterator_new(token_list, KS_CHARP);
  while ((curr = ks_iterator_get(iter)) != NULL)
  {
    struct auth_token* at = curr->vp;
    if (strcmp(at->user_name, uname) == 0)
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
  while ((curr = (ks_datacont*) ks_iterator_get(iter)) != NULL)
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
  while ((curr = ks_iterator_get(iter)) != NULL)
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
  while ((curr = (ks_datacont*) ks_iterator_get(iter)) != NULL)
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
  while ((curr = (ks_datacont*) ks_iterator_get(iter)) != NULL)
  {
    ks_list_remove_at(token_list, curr->i);
  }
  ks_iterator_delete(iter);
  ks_list_delete(dellist);
}
