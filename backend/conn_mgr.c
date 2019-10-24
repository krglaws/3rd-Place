
#include <stdlib.h>

#include "include/conn_mgr.h"


static struct connection_list cl = { 0, NULL };


void add_connection(int fd)
{
  struct connection* con = cl.head;
  struct connection* new_con = calloc(1, sizeof(struct connection));

  new_con->fd = fd;

  if (cl.head == NULL) 
  {
    cl.head = new_con;
    cl.count++;
    return; 
  }
  
  while (con->next) con = con->next;
  con->next = new_con;
  cl.count++;
}


void remove_connection(int fd)
{
  struct connection* con = cl.head;
  struct connection* temp;

  if (cl.head->fd == fd)
  {
    cl.head = cl.head->next;
    free(con);
    cl.count--;
    return;
  }

  while (con->next)
  {
    if (con->next->fd == fd)
    {
      temp = con->next;
      con->next = temp->next;
      free(temp);
      cl.count--;
      return;
    }
    con = con->next;
  }
}


int get_nth_fd(int n)
{
  if (n < 0) return 0;
  struct connection* con = cl.head;
  while (n-- > 0 && con) con = con->next;
  return con->fd;
}


/* returns highest fd value */
int initialize_fdset(fd_set* fds)
{
  int max = 0, fd;
  FD_ZERO(fds);
  for (int i = 0; i < cl.count; i++)
  {
    fd = get_nth_fd(i);
    FD_SET(fd, fds);
    if (max < fd) max = fd;
  }
  return max; 
}


int get_active_fd(fd_set* fds)
{
  int curr_fd;
  for (int i = 0; i < cl.count; i++)
  {
    curr_fd = get_nth_fd(i);
    if (FD_ISSET(curr_fd, fds))
      return curr_fd;
  }
  return -1;
}

