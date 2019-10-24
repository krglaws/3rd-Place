
#ifndef _CONN_MGR_H_
#define _CONN_MGR_H_

struct connection
{
  int fd;
  struct connection* next;
};


struct connection_list
{
  int count;
  struct connection* head;
};


void add_connection(int fd);


void remove_connection(int fd);


int get_nth_fd(int n);


int initialize_fdset(fd_set* fds);


int get_active_fd(fd_set* fds);


#endif

