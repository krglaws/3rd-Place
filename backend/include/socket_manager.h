#ifndef _SOCKET_MANAGER_H_
#define _SOCKET_MANAGER_H_

#include <inttypes.h>

/* Creates server socket and binds to specified address. Also initializes
    socket list. */
void init_socket_manager(const struct in6_addr* server_addr, const uint16_t server_port, const int max_clients);


/* Closes all sockets. */
void terminate_socket_manager();


/* Fills ipstr with the ip address belonging to sock */
void get_socket_ip(const int sock, char* ipstr, const int iplen);


/* Adds a new socket to socket list. */
void add_socket(int sock);


/* Closes and removes socket from socket list. */
void remove_socket(int sock);


/* Places all sockets from socket list into global
    fdset struct, then returns the value of the highest
    socket number (for use in select()).*/
static const int reload_socket_set();


/* Consults socket set to find the active socket following
    a call to select(). */
static const int get_active_socket();


/* Blocking call (via select()) that waits for socket activity.
    If active socket is server socket, new client socket is
    created and stored in socket list. */
const int await_active_socket();

#endif
