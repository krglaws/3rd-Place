#ifndef SOCKET_MANAGER_H
#define SOCKET_MANAGER_H

#include <inttypes.h>

#define MAX_ADDR_LEN (45)

struct sock_entry
{
  int sock;
  time_t  last_active;
  char addr[MAX_ADDR_LEN + 1];
};


/* Creates server socket and binds to specified address. Also initializes
    socket list. */
void init_socket_manager(const struct in6_addr* server_addr, const uint16_t server_port, const int max_clients);


/* Closes and removes socket from socket list. */
void remove_socket(struct sock_entry* se);


/* Blocking call (via select()) that waits for socket activity.
    If active socket is server socket, new client socket is
    created and stored in socket list. */
struct sock_entry* await_active_socket();

#endif
