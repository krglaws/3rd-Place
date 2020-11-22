
#include <stdio.h>
#include <stdlib.h>

#include <util.h>

char* load_file(const char* path)
{
  int filelen;
  FILE* fd;

  if ((fd = fopen(path, "r")) == NULL)
  {
    perror("http_get(): failed to open file");
    return NULL;
  }

  fseek(fd, 0, SEEK_END);
  filelen = ftell(fd);
  rewind(fd);

  char* content = malloc((filelen + 1) * sizeof(char));

  for (int i = 0; i < filelen; i++)
  {
    content[i] = fgetc(fd);
  }
  content[filelen] = '\0';
 
  fclose(fd);

  return content;
}
