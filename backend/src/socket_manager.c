#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <kylestructs.h>

#include <log_manager.h>
#include <socket_manager.h>


/* NOTES:
 * The sockets are currently stored in kylestructs ks_list,
 * but they should really be stored in custom linked ks_list
 * with a struct that holds both socket number and the
 * address associated with that socket. It will make it
 * much less annoying to retrieve the addresses of each
 * socket (i.e. I wont have to use get_socket_ip()).
 */


static int server_socket = 0;

static ks_list* socket_list = NULL;

static fd_set socket_set;


void init_socket_manager(const struct in6_addr* server_addr, const uint16_t server_port, const int max_clients)
{
  log_info("Initializing Socket Manager...");

  socket_list = ks_list_new();

  struct sockaddr_in6 addr;
  int addrlen = sizeof(addr);

  memset(&addr, 0, addrlen);
  addr.sin6_family = AF_INET6;
  addr.sin6_port = server_port;
  addr.sin6_addr = *(server_addr);

  if ((server_socket = socket(AF_INET6, SOCK_STREAM, 0)) == -1)
  {
    log_crit("init_socket_manager(): failed on call to socket(): ", strerror(errno));
  }

  int on = 1;
  if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (void*) &on, sizeof(on)) == -1)
  {
    log_crit("init_socket_manager(): setsockopt(): ", strerror(errno));
  }

  if (bind(server_socket, (struct sockaddr *) &addr, addrlen) == -1)
  {
    log_crit("init_socket_manager(): failed on call to bind(): ", strerror(errno));
  }

  if (listen(server_socket, max_clients) == -1)
  {
    log_crit("init_socket_manager(): failed on call to listen(): ", strerror(errno));
  }

  char ipstr[64];
  if (inet_ntop(addr.sin6_family, &(addr.sin6_addr), ipstr, sizeof(ipstr)) == NULL)
  {
    log_crit("init_socket_manager(): inet_ntop(): %s", strerror(errno));
  }

  log_info("Listening on address %s, port %d...", ipstr, ntohs(server_port));
}


void terminate_socket_manager()
{
  log_info("Terminating Socket Manager...");

  if (server_socket != 0)
  {
    close(server_socket);
  }

  int num_sockets = ks_list_length(socket_list);
  for (int i = 0; i < num_sockets; i++)
  {
    close(ks_list_get(socket_list, i)->i);
  }

  ks_list_delete(socket_list);
}


int get_socket_ip(const int sock, char* ipstr, const int iplen)
{
  struct sockaddr_in6 addr;
  int addrlen = sizeof(addr);

  // find out socket address
  if (getpeername(sock, (struct sockaddr*) &addr, &addrlen) == -1)
  {
    log_err("get_socket_ip(): getpeername(): %s", strerror(errno));
    return -1;
  }

  if (inet_ntop(addr.sin6_family, &(addr.sin6_addr), ipstr, iplen) == NULL)
  {
    log_err("get_socket_ip(): inet_ntop(): %s", strerror(errno));
    return -1;
  }

  return 0;
}


void add_socket(int sock)
{
  char ipstr[64];
  if (get_socket_ip(sock, ipstr, sizeof(ipstr)) == -1)
  {
    log_err("add_socket(): failed on call to get_socket_ip()" \
              "\nFailed to add new socket: %d", sock);
    close(sock);
    return;
  }

  log_info("Connected to %s (socket no. %d)", ipstr, sock);

  ks_list_add(socket_list, ks_datacont_new(&sock, KS_INT, 1));
}


void remove_socket(int sock)
{
  ks_datacont* dc = ks_datacont_new(&sock, KS_INT, 1);
  if (ks_list_remove_by(socket_list, dc) == -1)
  {
    log_err("remove_socket(): socket no. %d does not exist", sock);
    return;
  }
  ks_datacont_delete(dc);

  char ipstr[64];
  if (get_socket_ip(sock, ipstr, sizeof(ipstr)) == -1)
  {
    log_err("remove_socket(): failed on call to get_socket_ip()");
    log_info("Connection to [UNKNOWN] closed (socket no. %d)", sock);
    close(sock);
    return;
  }

  close(sock);
  log_info("Connection to %s closed (socket no. %d)", ipstr, sock);
}


static const int reload_socket_set()
{
  // clear socket set, add server socket
  FD_ZERO(&socket_set);
  FD_SET(server_socket, &socket_set);

  int max = server_socket;
  int num_sockets = ks_list_length(socket_list);

  // add sockets in socket ks_list to socket set
  for (int i = 0; i < num_sockets; i++)
  {
    ks_datacont* dc = ks_list_get(socket_list, i);
    FD_SET(dc->i, &socket_set);

    // find maximum socket no.
    if (max < dc->i) 
    {
      max = dc->i;
    }
  }

  return max; 
}


static const int get_active_socket()
{
  int num_sockets = ks_list_length(socket_list);

  // check if server socket is active
  if (FD_ISSET(server_socket, &socket_set))
  {
    return server_socket;
  }

  // look for active socket in ks_list
  for (int i = 0; i < num_sockets; i++)
  {
    ks_datacont* dc = ks_list_get(socket_list, i);
    if (FD_ISSET(dc->i, &socket_set))
    {
      int sock = dc->i;
      return sock;
    }
  }

  return -1;
}


const int await_active_socket()
{
  int max_socket = reload_socket_set();
  int active_socket, new_socket;

  struct sockaddr addr;
  int addrlen = sizeof(addr);
  char ipstr[64];

  // wait for active socket
  if (select(max_socket + 1, &socket_set, NULL, NULL, NULL) == -1)
  {
    log_crit("await_active_socket(): select(): %s", strerror(errno));
  }

  // find out which one is active
  if ((active_socket = get_active_socket()) == -1)
  {
    log_crit("await_active_socket(): no active socket");
  }

  // if server socket is active, there is a new connection
  if (active_socket == server_socket)
  {
    // accept connection
    if ((new_socket = accept(server_socket, (struct sockaddr*) &addr, &addrlen)) == -1)
    {
      log_crit("await_active_socket(): accept(): %s", strerror(errno));
    }

    // set recv() timeout to 1s
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;

    if (setsockopt(new_socket, SOL_SOCKET, SO_RCVTIMEO, (const void*)&tv, sizeof(tv)) == -1) {
      log_crit("await_active_socket(): setsockopt(): %s", strerror(errno));
    }

    add_socket(new_socket);

    return new_socket;
  }

  // otherwise, just return existing socket
  return active_socket;
}
