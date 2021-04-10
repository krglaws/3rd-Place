#ifndef LOAD_FILE_H
#define LOAD_FILE_H

#include <time.h>

struct file_pkg
{
  int length;
  char* contents;
};

struct cached_file
{
  time_t timestamp;
  int length;
  char* contents;
};

void init_file_manager();

struct file_pkg* load_file(const char* path);

#endif
