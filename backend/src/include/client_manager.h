
#ifndef _CONN_MGR_H_
#define _CONN_MGR_H_

void add_connection(int fd);

void remove_connection(int fd);

int initialize_fdset(fd_set* fds);

int get_active_fd(fd_set* fds);

#endif

