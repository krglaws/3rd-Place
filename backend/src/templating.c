
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "include/templating.h"


int check_braces(char* content)
{
  int len = strlen(content);
  int depth = 0;
  int pairs = 0;

  for (int i = 0; i < len; i++)
  {
    if (content[i] == '{') depth++;
    else if (content[i] == '}') {depth--; pairs++;}
    if (depth < 0 || depth > 1) return -1;
  }
  return pairs;
}


void fill_pair_list(char* contents, int* pair_list)
{
  int len = strlen(contents);
  for (int i = 0; i < len; i++)
    if (contents[i] == '{')
    {
      *pair_list = i;
      pair_list++;
    }
    else if (contents[i] == '}')
    {
      *pair_list = i;
      pair_list++;
    }
}


char* replace_with(char* contents, char* replace, char* with)
{
  int num_pairs;
  if ((num_pairs = check_braces(contents)) < 1)
    return NULL;

  int pairs[2*num_pairs];
  int result_size = strlen(contents) + (num_pairs * strlen(with));
  char* result = malloc(result_size);
  memset(result, 0, result_size);

  /* identify all brace pairs */
  fill_pair_list(contents, pairs);

  /* replace occurances of 'replace' string */
  int open, close, last_close = 0;
  size_t bytes;
  char* result_end = result;
  char* contents_end = contents;
  int num_swaps = 0;

  for (int i = 0; i < (num_pairs * 2); i+=2)
  {
    open = pairs[i];
    close = pairs[i+1];

    bytes = (open - last_close);
    memcpy(result_end, contents_end, bytes);
    result_end += bytes;
    contents_end += bytes;

    if (strncmp(replace, contents_end, (close - open)+1) == 0)
    {
      memcpy(result_end, with, strlen(with));
      result_end += strlen(with);
      num_swaps++;
    }
    else 
    {
      memcpy(result_end, contents_end, (close - open) + 1);
      result_end += (close - open) + 1;
    }
    contents_end = &(contents[close+1]);
    last_close = close + 1;
  }

  memcpy(result_end, contents_end, strlen(contents_end));
  int actual_size = (num_swaps * strlen(with)) + (strlen(contents) - (num_swaps * strlen(replace)));
  char* final = calloc(1, actual_size);
  memcpy(final, result, actual_size);

  free(result);
  free(contents);

  return final;
}


datacont* replace_all(datacont* html, list* replace, list* with)
{
  int count = list_length(replace);

  char init[html->size + 1];
  memcpy(init, html->cp, html->size);

  char* result = html->cp;

  for (int i = 0; i < count; i++)
  {
    result = replace_with(result, list_get(replace, i), list_get(with, i));
  }
}

