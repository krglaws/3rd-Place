#ifndef _TOKEN_MANGER_H_
#define _TOKEN_MANAGER_H_


/* removes a token_entry from token list at index */
void remove_token(const int index);


/* returns user info associated with cookie if present, else NULL */
const struct token_entry* valid_token(const char* token);


/* iterates over all tokens in list, removes them if days == 0,
   decrements the rest */
void check_tokens();


/* creates and returns a new token belonging to uname */
const char* new_token(const char* user_name, const char* user_id);


#endif