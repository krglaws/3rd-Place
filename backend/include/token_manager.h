
#ifndef _TOKEN_MANGER_H_
#define _TOKEN_MANAGER_H_

#define TOKENLEN (17)
#define TOKENLIFESPAN (7)

struct token_entry
{
  char token[TOKENLEN];
  int days;
  unsigned int user_id;
  struct token_entry* next;
};


/* removes a token_entry from token list at index */
void remove_token(const int index);


/* returns user_id belonging to cookie if present, else -1 */
long int valid_token(const char* token);


/* iterates over all tokens in list, removes them if days == 0,
   decrements the rest */
void check_tokens();


/* creates and returns a new token belonging to user_id */
char* new_token(const unsigned int user_id);

#endif

