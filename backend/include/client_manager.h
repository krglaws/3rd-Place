#ifndef CLIENT_MANAGER_H
#define CLIENT_MANAGER_H

#include <inttypes.h>

#define MAX_ADDR_LEN (45)
#define REQBUFSIZE (1024)
#define MAXREQSIZE (1024 * 1024)

struct client_entry
{
  int sock;
  char* raw_request;
  int content_length;
  time_t  last_active;
  char addr[MAX_ADDR_LEN + 1];
};


/* Creates server socket and binds to specified address. Also initializes
    client list. */
void init_client_manager(const struct in6_addr* server_addr, const uint16_t server_port, const int max_clients);


/* Closes connection to client and removes from client list. */
void remove_client(struct client_entry* se);


/* Blocking call (via select()) that waits for active client socket.
    If active socket is server socket, new client entry is
    created and stored in client list. */
struct client_entry* await_active_client();

#endif
