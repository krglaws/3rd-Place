#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <time.h>
#include <kylestructs.h>

#include <log_manager.h>
#include <socket_manager.h>

static struct sock_entry* add_socket(int sock);
static int reload_socket_set();
static struct sock_entry* get_active_socket();
static void terminate_socket_manager();

static struct sock_entry server_socket = {0};

static ks_list* socket_list = NULL;

static fd_set socket_set;


void init_socket_manager(const struct in6_addr* server_addr, const uint16_t server_port, const int max_clients)
{
  socket_list = ks_list_new();

  struct sockaddr_in6 addr;
  socklen_t addrlen = sizeof(addr);

  memset(&addr, 0, addrlen);
  addr.sin6_family = AF_INET6;
  addr.sin6_port = server_port;
  addr.sin6_addr = *(server_addr);

  if ((server_socket.sock = socket(AF_INET6, SOCK_STREAM, 0)) == -1)
  {
    log_crit("init_socket_manager(): failed on call to socket(): %s", strerror(errno));
  }

  int on = 1;
  if (setsockopt(server_socket.sock, SOL_SOCKET, SO_REUSEADDR, (void*) &on, sizeof(on)) == -1)
  {
    log_crit("init_socket_manager(): setsockopt(): %s", strerror(errno));
  }

  if (bind(server_socket.sock, (struct sockaddr *) &addr, addrlen) == -1)
  {
    log_crit("init_socket_manager(): failed on call to bind(): %s", strerror(errno));
  }

  if (listen(server_socket.sock, max_clients) == -1)
  {
    log_crit("init_socket_manager(): failed on call to listen(): %s", strerror(errno));
  }

  char ipstr[64];
  if (inet_ntop(addr.sin6_family, &(addr.sin6_addr), ipstr, sizeof(ipstr)) == NULL)
  {
    log_crit("init_socket_manager(): inet_ntop(): %s", strerror(errno));
  }

  atexit(&terminate_socket_manager);

  log_info("Listening on address %s, port %d...", ipstr, ntohs(server_port));
}


static void terminate_socket_manager()
{
  if (server_socket.sock != 0)
  {
    close(server_socket.sock);
  }

  struct sock_entry* se;
  ks_datacont* curr;
  ks_iterator* iter = ks_iterator_new(socket_list, KS_LIST);
  while ((curr = (ks_datacont*) ks_iterator_next(iter)) != NULL)
  {
    se = curr->vp;
    close(se->sock);
    free(se);
  }

  ks_iterator_delete(iter);
  ks_list_delete(socket_list);
}


static struct sock_entry* add_socket(int sock)
{
  struct sockaddr_in6 addr;
  socklen_t addrlen = sizeof(addr);

  struct sock_entry* se = malloc(sizeof(struct sock_entry));
  se->sock = sock;
  se->last_active = time(NULL);

  // find out socket address
  if (getpeername(sock, (struct sockaddr*) &addr, &addrlen) == -1)
  {
    log_err("add_socket(): getpeername(): %s", strerror(errno));
    close(sock);
    free(se);
    return NULL;
  }

  if (inet_ntop(addr.sin6_family, &(addr.sin6_addr), se->addr, sizeof(se->addr)) == NULL)
  {
    log_err("add_socket(): inet_ntop(): %s", strerror(errno));
    close(sock);
    free(se);
    return NULL;
  }

  ks_list_add(socket_list, ks_datacont_new(se, KS_VOIDP, 1));
  log_info("Connected to %s (socket number %d)", se->addr, sock);

  return se;
}


void remove_socket(struct sock_entry* se)
{
  int sock = se->sock;

  ks_datacont* dc = ks_datacont_new(se, KS_VOIDP, 1);
  if (ks_list_remove_by(socket_list, dc) == -1)
  {
    // this should never happen
    log_err("remove_socket(): socket entry with fd number %d does not exist", sock);
    ks_datacont_delete(dc);
    return;
  }
  ks_datacont_delete(dc);

  close(sock);
  log_info("Connection to %s closed (socket number %d)", se->addr, sock);
  free(se);
}


static int reload_socket_set()
{
  // clear socket set, add server socket
  FD_ZERO(&socket_set);
  FD_SET(server_socket.sock, &socket_set);

  int max = server_socket.sock;

  struct sock_entry* se;
  const ks_datacont* curr;
  ks_iterator* iter = ks_iterator_new(socket_list, KS_LIST);
  while ((curr = ks_iterator_next(iter)) != NULL)
  {
    se = curr->vp;

    // if it has been 1 hour since last request, close
    int delta = time(NULL) - se->last_active;
    if (delta > 3600)
    {
      remove_socket(se);
      continue;
    }

    FD_SET(se->sock, &socket_set);

    // find maximum socket number
    if (max < se->sock)
    {
      max = se->sock;
    }
  }
  ks_iterator_delete(iter);

  return max; 
}


static struct sock_entry* get_active_socket()
{
  // check if server socket is active
  if (FD_ISSET(server_socket.sock, &socket_set))
  {
    return &server_socket;
  }

  // look for active socket in ks_list
  const ks_datacont* curr;
  ks_iterator* iter = ks_iterator_new(socket_list, KS_LIST);
  while ((curr = ks_iterator_next(iter)) != NULL)
  {
    struct sock_entry* se = curr->vp;
    if (FD_ISSET(se->sock, &socket_set))
    {
      ks_iterator_delete(iter);
      se->last_active = time(NULL);
      return se;
    }
  }
  ks_iterator_delete(iter);

  return NULL;
}


struct sock_entry* await_active_socket()
{
  int max_socket = reload_socket_set();
  struct sock_entry *active_socket;
  int new_socket;

  struct sockaddr addr;
  socklen_t addrlen = sizeof(addr);

  // wait for active socket
  if (select(max_socket + 1, &socket_set, NULL, NULL, NULL) == -1)
  {
    log_crit("await_active_socket(): select(): %s", strerror(errno));
  }

  // find out which one is active
  if ((active_socket = get_active_socket()) == NULL)
  {
    log_err("await_active_socket(): no active socket");
    return NULL;
  }

  // if server socket is active, there is a new connection
  if (active_socket == &server_socket)
  {
    // accept connection
    if ((new_socket = accept(server_socket.sock, (struct sockaddr*) &addr, &addrlen)) == -1)
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

    return add_socket(new_socket);
  }

  // otherwise, just return existing socket
  return active_socket;
}
