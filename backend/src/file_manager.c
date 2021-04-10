#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <string_map.h>
#include <log_manager.h>
#include <file_manager.h>


static ks_hashmap* file_map = NULL;
static void terminate_file_manager();

void init_file_manager()
{
  file_map = ks_hashmap_new(KS_CHARP, 32);
  atexit(&terminate_file_manager);
}

static void terminate_file_manager()
{
  // need to free each cached_file struct individually
  // since ks_hashmap_delete can't delete void pointers
  int count = ks_hashmap_count(file_map);

  for (int i = 0; i < count; i++)
  {
    const ks_datacont* key_dc = ks_hashmap_get_key(file_map, i);
    struct cached_file* cf = get_map_value_vp(file_map, key_dc->cp);
    free(cf->contents);
    free(cf);
  }

  ks_hashmap_delete(file_map);
}


struct file_pkg* load_file(const char* path)
{
  struct stat status;
  bool exists = stat(path, &status) == 0;

  struct cached_file* cf;
  if ((cf = get_map_value_vp(file_map, path)) != NULL)
  {
    // if its in the map but not available in the file system, delete cached copy
    // and return NULL
    if (exists == false)
    {
      log_err("load_file(): failed to stat cached file: %s", path);
      remove_map_value(file_map, path);
      return NULL;
    }

    // check if no refresh is needed
    if (cf->timestamp == status.st_mtime)
    {
      // package
      struct file_pkg* pkg = malloc(sizeof(struct file_pkg));
      pkg->length = cf->length;
      pkg->contents = malloc(sizeof(char) * (cf->length + 1));
      memcpy(pkg->contents, cf->contents, cf->length+1);
      return pkg;
    }
  }

  if (exists == false)
  {
    return NULL;
  }

  // delete cache entry if cf not NULL
  if (cf != NULL)
  {
    free(cf->contents);
    free(cf);
  }

  // open file
  FILE* fd;
  if ((fd = fopen(path, "r")) == NULL)
  {
    log_err("load_file(): failed to open '%s': %s", path, strerror(errno));
    return NULL;
  }

  // get file length
  fseek(fd, 0, SEEK_END);
  int filelen = ftell(fd);
  rewind(fd);

  // read into buffer
  char* contents = malloc((sizeof(char) * filelen) + 1);
  fread(contents, 1, filelen, fd);
  contents[filelen] = '\0';
  fclose(fd);

  // build cache struct
  cf = malloc(sizeof(struct cached_file));
  cf->timestamp = status.st_mtime;
  cf->length = filelen;
  cf->contents = contents;

  add_map_value_vp(file_map, path, cf);

  return load_file(path);
}
