#ifndef VALIDATOR_H
#define VALIDATOR_H

int init_validator();

bool valid_name(const char* name, int max_len);

bool valid_content(char* dest, const char* src, int max_len);

#endif
