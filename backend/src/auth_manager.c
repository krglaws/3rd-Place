#define _XOPEN_SOURCE
#include <unistd.h>
#include <crypt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include <string_map.h>
#include <log_manager.h>
#include <sql_manager.h>
#include <auth_manager.h>


/* ks_list of login tokens */
static struct token_entry* head = NULL;


// file for reading from /dev/urandom for secure
// number generation.
static FILE* urandom = NULL;


// These are the 64 chars that can be used in the
// 'salt' string for the crypt function. Base64 
// might be kind of a misnomer since the order is wrong,
// and there's supposed to be a '+' instead of '.', but
// that doesnt really matter here.

// 64 chars used by random_salt and random_token
static const char base64[64] = "./0123456789ABCD" \
                               "EFGHIJKLMNOPQRST" \
                               "UVWXYZabcdefghij" \
                               "klmnopqrstuvwxyz";


void init_auth_manager()
{
  log_info("Initializing Auth Mananger...");

  if ((urandom = fopen("/dev/urandom", "r")) == NULL)
  {
    log_warn("Failed to open /dev/urandom: %s", strerror(errno));
    srand(time(NULL));
  }
}


static void delete_all_tokens(struct token_entry* t)
{
  if (t != NULL)
  {
    delete_all_tokens(t->next);
  }

  delete_token_entry(t);
}


void terminate_auth_manager()
{
  log_info("Terminating Auth Manager...");

  if (urandom != NULL)
  {
    fclose(urandom);
  }

  delete_all_tokens(head);
}


static char rand_byte()
{
  int index = (int) (urandom ? fgetc(urandom) : rand()) % 64;
  return base64[index];
}


static void random_salt(char* saltbuf)
{
  saltbuf[0] = rand_byte();
  saltbuf[1] = rand_byte();
  saltbuf[2] = '\0';

  return;
}


static ks_hashmap* get_user_info(const char* uname)
{
  ks_list* result = query_users_by_name(uname);

  ks_datacont* row0 = ks_list_get(result, 0);
  ks_datacont* row1 = ks_list_get(result, 1);

  // check if user exists
  if (row0 == NULL)
  {
    ks_list_delete(result);
    return NULL;
  }

  // NOTE:
  // might remove this check
  if (row1 != NULL)
  {
    log_crit("get_user_info(): multiple users with name %s", uname);
  }

  ks_hashmap* user_info = row0->hm;

  // remove reference to user_info so that
  // ks_list_delete() doesn't delete it.
  row0->hm = NULL;
  ks_list_delete(result);

  return user_info;
}


// just return reference to token
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
  const char* hash1 = get_map_value(user_info, FIELD_USER_PASSWORD_HASH)->cp;

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
    return NULL;
  }
  ks_hashmap_delete(user_info);

  // user is valid, retrieve token if already exists,
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


const char* new_user(const char* uname, const char* passwd)
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
  random_salt(salt);
  char* pwhash = crypt(passwd, salt);

  // prepare insert query
  if (insert_new_user(uname, pwhash) == -1)
  {
    log_err("new_user(): failed to create new user");
    return NULL;
  }

  const char* token;
  if ((token = new_token(uname)) == NULL)
  {
    log_err("new_user(): failed to create new user token");
    return NULL;
  }

  return token;
}


static void random_token(char* token_buf)
{
  for (int i = 0; i < TOKENLEN; i++)
  {
    token_buf[i] = rand_byte();
  }
}


static const char* new_token(const char* uname)
{
  // pull user info from database
  ks_hashmap* user_info = get_user_info(uname);
  const char* user_id = get_map_value(user_info, FIELD_USER_ID)->cp;

  // copy info into auth token
  struct auth_token* new_token = calloc(1, sizeof(struct auth_token));
  memcpy(new_token->user_id, user_id, strlen(user_id));
  memcpy(new_token->user_name, uname, strlen(uname));
  ks_hashmap_delete(user_info);

  // create new token entry
  struct token_entry* new_entry = calloc(1, sizeof(struct token_entry));
  new_entry->token = new_token;
  new_entry->days = TOKENLIFESPAN;

  // generate random token
  do
  {
    random_token(new_token->token);
  } while (valid_token(new_token->token) != NULL);

  // append token entry to ks_list
  if (head == NULL)
  {
    head = new_entry;
  }
  else
  {
    struct token_entry* tail = head;
    while (tail->next != NULL)
    {
      tail = tail->next;
    }
    tail->next = new_entry;
  }

  // return token string
  return new_token->token;
}


static const char* get_token(const char* uname)
{
  if (uname == NULL)
  {
    return NULL;
  }

  struct token_entry* iter = head;

  while (iter != NULL)
  {
    if (strcmp(iter->token->user_name, uname) == 0)
    {
      return iter->token->token;
    }
    iter = iter->next;
  }

  return NULL;
}


// called by http_post() for logout
void remove_token(const char* token)
{
  if (token == NULL)
  {
    log_err("remove_token(): null token argument");
    return;
  }

  struct token_entry* iter;

  // check head
  if (strcmp(head->token->token, token) == 0)
  {
    iter = head->next;
    delete_token_entry(head);
    head = iter;
    return;
  }

  iter = head;

  // check the rest of the ks_list
  while (iter->next != NULL)
  {
    if (strcmp(iter->next->token->token, token) == 0)
    {
      struct token_entry* temp = iter->next->next;
      delete_token_entry(iter->next);
      iter->next = temp;
      return;
    }

    iter = iter->next;
  }

  log_err("remove_token(): failed to find matching token string");
}


/* returns auth_token struct that contains token if present, else NULL */
const struct auth_token* valid_token(const char* token)
{
  if (token == NULL)
  {
    return NULL;
  }

  struct token_entry* iter = head;

  while (iter != NULL)
  {
    if (strcmp(iter->token->token, token) == 0)
    {
      // user has visited site, so restore token lifespan
      iter->days = TOKENLIFESPAN;
      return iter->token;
    }
    iter = iter->next;
  }

  /* token not found */
  return NULL;
}


/* iterates over all token in ks_list, decrements days,
   removes if days < 1 */
void check_tokens()
{
  if (head == NULL)
  {
    return;
  }

  struct token_entry* iter;

  // check if head node is toast
  int done = 1;
  do
  {
    head->days--;

    if (head->days < 1)
    {
      done = 0;
      iter = head->next;
      delete_token_entry(head);
      head = iter;
    }
    else
    {
      done = 1;
    }
  } while (!done);

  iter = head;

  // now iterate over ks_list with iterator behind one
  while (iter->next != NULL)
  {
    iter->next->days--;

    if (iter->next->days == 0)
    {
      struct token_entry* temp = iter->next->next;
      delete_token_entry(iter->next);
      iter->next = temp;
    }
    else
    {
      iter = iter->next;
    }
  }
}


void delete_token_entry(struct token_entry* t)
{
  if (t == NULL)
  {
    return;
  }

  free(t->token);
  free(t);
}
