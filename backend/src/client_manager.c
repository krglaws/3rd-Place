
#include <stdlib.h>
#include <unistd.h>
#include <kylestructs.h>

#include <client_manager.h>

/* client socket list */
static list* client_list;

/* current number of client sockets */
static int num_clients = 0;


void init_client_manager()
{
  client_list = list_new();
}


void terminate_client_manager()
{
  for (int i = 0; i < list_length(client_list); i++)
  {
    close(list_get(client_list, i)->i);
  }
  list_delete(client_list);
}


void add_client(int fd)
{
  list_add(client_list, datacont_new(&fd, INT, 1));

  num_clients++;
}


void remove_client(int fd)
{
  close(fd);

  datacont* dc = datacont_new(&fd, INT, 1);

  list_remove_by(client_list, dc);

  datacont_delete(dc);

  num_clients--;
}


int initialize_fdset(fd_set* fds)
{
  int max = 0;
  datacont* dc;

  FD_ZERO(fds);

  for (int i = 0; i < num_clients; i++)
  {
    dc = list_get(client_list, i);
    FD_SET(dc->i, fds);
    if (max < dc->i) max = dc->i;
  }
  return max; 
}


int get_active_client(fd_set* fds)
{
  for (int i = 0; i < num_clients; i++)
  {
    datacont* dc = list_get(client_list, i);
    if (FD_ISSET(dc->i, fds))
    {
      int fd = dc->i;
      return fd;
    }
  }
  return -1;
}


