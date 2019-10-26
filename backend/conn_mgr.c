#include <stdio.h>
#include <stdlib.h>
#include <kylestructs/datacont.h>
#include <kylestructs/listnode.h>

#include "include/conn_mgr.h"


static listnode* conns = NULL;
static int num_conns = 0;


void add_connection(int fd)
{
  if (conns == NULL)
    conns = listnode_new(datacont_new(&fd, INT, 1));

  else listnode_add(conns, datacont_new(&fd, INT, 1));

  num_conns++;
}


void remove_connection(int fd)
{
  datacont* dc = datacont_new(&fd, INT, 1);

  listnode_remove(conns, dc);

  datacont_delete(dc);

  if (--num_conns == 0)
    conns = NULL;
}


int initialize_fdset(fd_set* fds)
{
  int max = 0;
  datacont* dc;

  FD_ZERO(fds);

  for (int i = 0; i < num_conns; i++)
  {
    dc = listnode_get_nth(conns, i);
    FD_SET(dc->i, fds);
    if (max < dc->i) max = dc->i;
    datacont_delete(dc);
  }
  return max; 
}


int get_active_fd(fd_set* fds)
{
  datacont* dc;
  for (int i = 0; i < num_conns; i++)
  {
    dc = listnode_get_nth(conns, i);
    if (FD_ISSET(dc->i, fds))
    {
      int fd = dc->i;
      datacont_delete(dc);
      return fd;
    }
    datacont_delete(dc);
  }
  return -1;
}

