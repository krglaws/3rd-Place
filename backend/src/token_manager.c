
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <token_manager.h>

/* list of active tokens */
static struct token_entry* head = NULL;


/* remove token by index */
void remove_token(const int index)
{
  struct token_entry* deletethis;
  if (index == 0)
  {
    deletethis = head;
    head = head->next;
    free(deletethis);
    return;
  }

  int i = 0;
  struct token_entry* temp = head;

  while (i < index-1)
  {
    i++;
    temp = temp->next;
  }

  deletethis = temp->next;
  temp->next = temp->next->next;
  free(deletethis);
}


/* returns user_id belonging to token if present,
   else -1 */
long int valid_token(const char* token)
{
  struct token_entry* iter = head;

  while (iter != NULL)
  {
    if (strcmp(token, iter->token) == 0)
    {
      return iter->user_id;
    }
    iter = iter->next;
  }

  /* token not found */
  return -1;
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


static void random_token(char* token_buff)
{
  static const char hexmap[16] = 
  {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

  for (int i = 0; i < TOKENLEN; i++)
  {
    token_buff[i] = hexmap[rand() % 16];
  }
}


/* creates and returns a new token belonging to user_id */
char* new_token(const unsigned int user_id)
{
  struct token_entry* new_entry = calloc(1, sizeof(struct token_entry));
  new_entry->days = TOKENLIFESPAN;
  new_entry->user_id = user_id;

  do
  {
    random_token(new_entry->token);
  } while (valid_token(new_entry->token) != -1);

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

  return new_entry->token;
}

