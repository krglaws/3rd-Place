#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <kylestructs.h>

#include <log_manager.h>
#include <socket_manager.h>


static int server_socket = 0;

static list* socket_list = NULL;

static fd_set socket_set;


void init_socket_manager(const struct in6_addr* server_addr, const uint16_t server_port, const int max_clients)
{
  log_info("Initializing Socket Manager...");

  socket_list = list_new();

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

  int num_sockets = list_length(socket_list);
  for (int i = 0; i < num_sockets; i++)
  {
    close(list_get(socket_list, i)->i);
  }

  list_delete(socket_list);
}


void get_socket_ip(const int sock, char* ipstr, const int iplen)
{
  struct sockaddr_in6 addr;
  int addrlen = sizeof(addr);

  // find out socket address
  if (getpeername(sock, (struct sockaddr*) &addr, &addrlen) == -1)
  {
    log_crit("get_socket_ip(): getpeername(): %s", strerror(errno));
  }

  if (inet_ntop(addr.sin6_family, &(addr.sin6_addr), ipstr, iplen) == NULL)
  {
    log_crit("get_socket_ip(): inet_ntop(): %s", strerror(errno));
  }
}


void add_socket(int sock)
{
  char ipstr[64];
  get_socket_ip(sock, ipstr, sizeof(ipstr));

  list_add(socket_list, datacont_new(&sock, INT, 1));

  log_info("Connected to %s (socket no. %d)", ipstr, sock);
}


void remove_socket(int sock)
{
  char ipstr[64];
  get_socket_ip(sock, ipstr, sizeof(ipstr));

  datacont* dc = datacont_new(&sock, INT, 1);
  list_remove_by(socket_list, dc);
  datacont_delete(dc);

  close(sock);

  log_info("Connection to %s closed (socket no. %d)", ipstr, sock);
}


static const int reload_socket_set()
{
  FD_ZERO(&socket_set);

  int max = 0;
  int num_sockets = list_length(socket_list);

  for (int i = 0; i < num_sockets; i++)
  {
    datacont* dc = list_get(socket_list, i);
    FD_SET(dc->i, &socket_set);

    if (max < dc->i) 
    {
      max = dc->i;
    }
  }

  return max; 
}


static const int get_active_socket()
{
  int num_sockets = list_length(socket_list);

  for (int i = 0; i < num_sockets; i++)
  {
    datacont* dc = list_get(socket_list, i);
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
