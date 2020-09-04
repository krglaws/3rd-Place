
#ifndef _TOKEN_MANGER_H_
#define _TOKEN_MANAGER_H_

#define TOKENLEN (17)
#define TOKENLIFESPAN (7)

struct token_entry
{
  char token[TOKENLEN];
  char uname[UNAMELEN];
  int days;
  struct token_entry* next;
};


/* removes a token_entry from token list at index */
void remove_token(const int index);


/* returns uname belonging to cookie if present, else NULL */
const char* valid_token(const char* token);


/* iterates over all tokens in list, removes them if days == 0,
   decrements the rest */
void check_tokens();


/* creates and returns a new token belonging to uname */
const char* new_token(char* uname);


#endif

