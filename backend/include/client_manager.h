
#ifndef _CONN_MGR_H_
#define _CONN_MGR_H_

void init_client_manager();

void terminate_client_manager();

void add_client(int fd);

void remove_client(int fd);

int initialize_fdset(fd_set* fds);

int get_active_client(fd_set* fds);

#endif

