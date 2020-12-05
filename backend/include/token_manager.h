
#ifndef _TOKEN_MANGER_H_
#define _TOKEN_MANAGER_H_

#define TOKENLEN (32)
#define TOKENLIFESPAN (7)

struct token_entry
{
  char token[TOKENLEN + 1];
  int uid;
  char uname[UNAMELEN + 1];
  int days;
  struct token_entry* next;
};


/* removes a token_entry from token list at index */
void remove_token(const int index);


/* returns user info associated with cookie if present, else NULL */
const struct token_entry* valid_token(const char* token);


/* iterates over all tokens in list, removes them if days == 0,
   decrements the rest */
void check_tokens();


/* creates and returns a new token belonging to uname */
const char* new_token(char* uname);


#endif

