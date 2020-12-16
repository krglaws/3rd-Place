
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <log_manager.h>
#include <util.h>


// load a file into a NULL terminated string
char* load_file(const char* path)
{
  int filelen;
  FILE* fd;

  if ((fd = fopen(path, "r")) == NULL)
  {
    log_err("load_file(): failed to open '%s': %s", path, strerror(errno));
    return NULL;
  }

  fseek(fd, 0, SEEK_END);
  filelen = ftell(fd);
  rewind(fd);

  char* file = malloc((sizeof(char) * filelen) + 1);

  for (int i = 0; i < filelen; i++)
  {
    file[i] = fgetc(fd);
  }
  file[filelen] = '\0';

  fclose(fd);

  return file;
}


// loads a file into a struct which also contains the file length.
// this is useful for files that may contain NULL bytes and thus
// cannot be NULL terminated.
struct file_str* load_file_str(const char* path)
{
  int filelen;
  FILE* fd;

  if ((fd = fopen(path, "r")) == NULL)
  {
    log_err("load_file_str(): failed to open '%s': %s", path, strerror(errno));
    return NULL;
  }

  fseek(fd, 0, SEEK_END);
  filelen = ftell(fd);
  rewind(fd);

  struct file_str* file = malloc(sizeof(struct file_str));
  file->contents = malloc(filelen * sizeof(char));
  file->len = filelen;

  for (int i = 0; i < filelen; i++)
  {
    file->contents[i] = fgetc(fd);
  }
 
  fclose(fd);

  return file;
}


void del_file_str(struct file_str* file)
{
  free(file->contents);
  free(file);
}
