
#include <stdio.h>
#include <stdlib.h>
#include <kylestructs.h>

#include <client_manager.h>


static list* clients;

static int num_conns = 0;


void init_conn_mgr()
{
  clients = list_new();
}


void add_connection(int fd)
{
  list_add(clients, datacont_new(&fd, INT, 1));

  num_conns++;
}


void remove_connection(int fd)
{
  datacont* dc = datacont_new(&fd, INT, 1);

  list_remove_by(clients, dc);

  datacont_delete(dc);

  num_conns--;
}


int initialize_fdset(fd_set* fds)
{
  int max = 0;
  datacont* dc;

  FD_ZERO(fds);

  for (int i = 0; i < num_conns; i++)
  {
    dc = list_get(clients, i);
    FD_SET(dc->i, fds);
    if (max < dc->i) max = dc->i;
  }
  return max; 
}


int get_active_fd(fd_set* fds)
{
  datacont* dc;
  for (int i = 0; i < num_conns; i++)
  {
    dc = list_get(clients, i);
    if (FD_ISSET(dc->i, fds))
    {
      int fd = dc->i;
      return fd;
    }
  }
  return -1;
}

