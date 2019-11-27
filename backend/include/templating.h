
#ifndef _TEMPLATING_H_
#define _TEMPLATING_H_

int check_braces(char* content);

void fill_pair_list(char* contents, int* pair_list);
	
char* insert_html(char* contents, char* replace, char* with);

char* load_html(char* fname);

#endif

