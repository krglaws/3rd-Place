#define _XOPEN_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>

#include <log_manager.h>
#include <sql_wrapper.h>
#include <auth_manager.h>


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


void terminate_auth_manager()
{
  log_info("Terminating Auth Manager...");

  if (urandom != NULL)
  {
    fclose(urandom);
  }
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


static list* get_user_info(const char* uname)
{
  char* query_fmt = QUERY_USER_BY_UNAME;
  char query[strlen(query_fmt) + strlen(uname) + 1];
  sprintf(query, query_fmt, uname);

  list** result = query_database_ls(query);

  // check if user exists
  if (result[0] == NULL)
  {
    delete_query_result(result);
    return NULL;
  }

  // NOTE:
  // might remove this check
  if (result[1] != NULL)
  {
    log_crit("get_user_info(): multiple users with name %s", uname);
  }

  list* user_info = result[0];
  free(result);

  return user_info;
}


// just return reference to token
const char* login_user(const char* uname, const char* passwd)
{
  if (uname == NULL || passwd == NULL)
  {
    return NULL;
  }

  list* user_info = get_user_info(uname);

  // grab user hash from query
  const char* hash1 = list_get(user_info, SQL_FIELD_USER_PASSWORD_HASH)->cp;

  char salt[3];
  salt[0] = hash1[0];
  salt[1] = hash1[1];
  salt[2] = '\0';

  // hash submittted passwd
  char* hash2;
  if ((hash2 = crypt(passwd, salt)) == NULL)
  {
    list_delete(user_info);
    return NULL;
  }

  // compare hashes
  if (strcmp(hash1, hash2) != 0)
  {
    list_delete(user_info);
    return NULL;
  }
  list_delete(user_info);

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
  list* user_info = get_user_info(uname);

  if (user_info != NULL)
  {
    list_delete(user_info);
    return NULL;
  }

  // hash user passwd
  char salt[3];
  random_salt(salt);
  char* pwhash = crypt(passwd, salt);

  // prepare insert query
  char *query_fmt = INSERT_NEW_USER;
  int querylen = strlen(query_fmt) + strlen(uname) + strlen(pwhash) + 1;
  char query[querylen];
  sprintf(query, query_fmt, uname, pwhash);

  // add new user to database
  list** result = query_database_ls(query);
  if (result != NULL)
  {
    delete_query_result(result);
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
  list* user_info = get_user_info(uname);
  const char* user_id = list_get(user_info, SQL_FIELD_USER_ID)->cp;

  // copy info into auth token
  struct auth_token* new_token = calloc(1, sizeof(struct auth_token));
  memcpy(new_token->user_id, user_id, strlen(user_id));
  memcpy(new_token->user_name, uname, strlen(uname));
  list_delete(user_info);

  // create new token entry
  struct token_entry* new_entry = calloc(1, sizeof(struct token_entry));
  new_entry->token = new_token;
  new_entry->days = TOKENLIFESPAN;

  // generate random token
  do
  {
    random_token(new_token->token);
  } while (valid_token(new_token->token) != NULL);

  // append token entry to list
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


/* remove token by index */
void remove_token(const int index)
{
  struct token_entry* deletethis;
  if (index == 0)
  {
    deletethis = head;
    head = head->next;
    free(deletethis->token);
    free(deletethis);
    return;
  }

  int i = 0;
  struct token_entry* temp = head;

  while (i < index-1)
  {
    if (temp == NULL)
    {
      log_crit("remove_token(): attempted to remove token at out of bounds index: %d", index);
    }
    i++;
    temp = temp->next;
  }

  deletethis = temp->next;
  temp->next = temp->next->next;
  free(deletethis);
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


/* iterates over all token in list, decrements days,
   removes if days < 1 */
void check_tokens()
{
  int index = 0;
  struct token_entry* iter = head;
  while (iter != NULL)
  {
    iter->days--;

    if (iter->days == 0)
    {
      remove_token(index);
    }

    index++;
    iter = iter->next;
  }
}
