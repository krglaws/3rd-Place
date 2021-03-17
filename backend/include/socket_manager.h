#ifndef SOCKET_MANAGER_H
#define SOCKET_MANAGER_H

#include <inttypes.h>

/* Creates server socket and binds to specified address. Also initializes
    socket list. */
void init_socket_manager(const struct in6_addr* server_addr, const uint16_t server_port, const int max_clients);


/* Fills ipstr with the ip address belonging to sock */
int get_socket_ip(const int sock, char* ipstr, const int iplen);


/* Adds a new socket to socket list. */
void add_socket(int sock);


/* Closes and removes socket from socket list. */
void remove_socket(int sock);


/* Blocking call (via select()) that waits for socket activity.
    If active socket is server socket, new client socket is
    created and stored in socket list. */
int await_active_socket();

#endif
