#ifndef _UTIL_H_
#define _UTIL_H_

char* load_file(const char* path);

struct file_str
{
  char* contents;
  int len;
};

struct file_str* load_file_str(const char* path);

void del_file_str(struct file_str* file);

#endif
