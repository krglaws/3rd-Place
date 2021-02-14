#ifndef LOAD_FILE_H
#define LOAD_FILE_H

char* load_file(const char* path);

struct file_str
{
  char* contents;
  int len;
};

struct file_str* load_file_str(const char* path);

void del_file_str(struct file_str* file);

#endif
