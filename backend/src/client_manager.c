#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <time.h>
#include <kylestructs.h>

#include <log_manager.h>
#include <client_manager.h>
#include <response.h>

static int add_client(int sock);
static int reload_socket_set();
static int read_client_request(struct client_entry* ce);
static struct client_entry* get_active_client();
static void terminate_client_manager();

static struct client_entry server_entry = {0};

static ks_list* client_list = NULL;

static fd_set socket_set;


void init_client_manager(const struct in6_addr* server_addr, const uint16_t server_port, const int max_clients)
{
  client_list = ks_list_new();

  struct sockaddr_in6 addr;
  socklen_t addrlen = sizeof(addr);

  memset(&addr, 0, addrlen);
  addr.sin6_family = AF_INET6;
  addr.sin6_port = server_port;
  addr.sin6_addr = *(server_addr);

  if ((server_entry.sock = socket(AF_INET6, SOCK_STREAM, 0)) == -1)
  {
    log_crit("init_client_manager(): failed on call to socket(): %s", strerror(errno));
  }

  int on = 1;
  if (setsockopt(server_entry.sock, SOL_SOCKET, SO_REUSEADDR, (void*) &on, sizeof(on)) == -1)
  {
    log_crit("init_client_manager(): setsockopt(): %s", strerror(errno));
  }

  if (bind(server_entry.sock, (struct sockaddr *) &addr, addrlen) == -1)
  {
    log_crit("init_client_manager(): failed on call to bind(): %s", strerror(errno));
  }

  if (listen(server_entry.sock, max_clients) == -1)
  {
    log_crit("init_client_manager(): failed on call to listen(): %s", strerror(errno));
  }

  char ipstr[64];
  if (inet_ntop(addr.sin6_family, &(addr.sin6_addr), ipstr, sizeof(ipstr)) == NULL)
  {
    log_crit("init_client_manager(): inet_ntop(): %s", strerror(errno));
  }

  atexit(&terminate_client_manager);

  log_info("Listening on address %s, port %d...", ipstr, ntohs(server_port));
}


static void terminate_client_manager()
{
  if (server_entry.sock != 0)
  {
    close(server_entry.sock);
  }

  struct client_entry* ce;
  ks_datacont* curr;
  ks_iterator* iter = ks_iterator_new(client_list, KS_LIST);
  while ((curr = (ks_datacont*) ks_iterator_next(iter)) != NULL)
  {
    ce = curr->vp;
    close(ce->sock);
    free(ce->raw_request);
    free(ce);
  }

  ks_iterator_delete(iter);
  ks_list_delete(client_list);
}


static int add_client(int sock)
{
  struct sockaddr_in6 addr;
  socklen_t addrlen = sizeof(addr);

  struct client_entry* ce = malloc(sizeof(struct client_entry));
  ce->sock = sock;
  ce->raw_request = NULL;
  ce->content_length = 0;
  ce->last_active = time(NULL);

  // find out socket address
  if (getpeername(sock, (struct sockaddr*) &addr, &addrlen) == -1)
  {
    log_err("add_client(): getpeername(): %s", strerror(errno));
    close(sock);
    free(ce);
    return -1;
  }

  if (inet_ntop(addr.sin6_family, &(addr.sin6_addr), ce->addr, sizeof(ce->addr)) == NULL)
  {
    log_err("add_client(): inet_ntop(): %s", strerror(errno));
    close(sock);
    free(ce);
    return -1;
  }

  ks_list_add(client_list, ks_datacont_new(ce, KS_VOIDP, 1));
  log_info("Connected to %s (socket number %d)", ce->addr, sock);

  return 0;
}


void remove_client(struct client_entry* ce)
{
  int sock = ce->sock;

  ks_datacont* dc = ks_datacont_new(ce, KS_VOIDP, 1);
  if (ks_list_remove_by(client_list, dc) == -1)
  {
    // this should never happen
    log_err("remove_client(): socket entry with fd number %d does not exist", sock);
    ks_datacont_delete(dc);
    return;
  }
  ks_datacont_delete(dc);

  close(sock);
  log_info("Connection to %s closed (socket number %d)", ce->addr, sock);
  free(ce->raw_request);
  free(ce);
}


static int reload_socket_set()
{
  // clear socket set, add server socket
  FD_ZERO(&socket_set);
  FD_SET(server_entry.sock, &socket_set);

  int max = server_entry.sock;

  struct client_entry* ce;
  const ks_datacont* curr;
  ks_iterator* iter = ks_iterator_new(client_list, KS_LIST);
  while ((curr = ks_iterator_next(iter)) != NULL)
  {
    ce = curr->vp;

    // if it has been 1 hour since last request, close
    int delta = time(NULL) - ce->last_active;
    if (delta > 3600)
    {
      remove_client(ce);
      continue;
    }

    FD_SET(ce->sock, &socket_set);

    // find maximum socket number
    if (max < ce->sock)
    {
      max = ce->sock;
    }
  }
  ks_iterator_delete(iter);

  return max; 
}


static struct client_entry* get_active_client()
{
  // check if server socket is active
  if (FD_ISSET(server_entry.sock, &socket_set))
  {
    return &server_entry;
  }

  // look for active socket in ks_list
  const ks_datacont* curr;
  ks_iterator* iter = ks_iterator_new(client_list, KS_LIST);
  while ((curr = ks_iterator_next(iter)) != NULL)
  {
    struct client_entry* ce = curr->vp;
    if (FD_ISSET(ce->sock, &socket_set))
    {
      ks_iterator_delete(iter);
      ce->last_active = time(NULL);
      return ce;
    }
  }
  ks_iterator_delete(iter);

  return NULL;
}


static int read_client_request(struct client_entry* ce)
{
  char req_buf[REQBUFSIZE + 1];
  int read_bytes = recv(ce->sock, req_buf, REQBUFSIZE, 0);
  if (read_bytes == -1)
  {
    log_err("read_client_request(): Failed on call to recv(): %s", strerror(errno));
    remove_client(ce);
    return -1;
  }

  if (read_bytes == 0)
  {
    remove_client(ce);
    return -1;
  }

  // copy request buffer into ce->raw_request
  int cpy_start = ce->raw_request ? strlen(ce->raw_request) : 0;
  if (cpy_start + read_bytes > MAXREQSIZE)
  {
    response_error(STAT413);
    remove_client(ce);
    return -1;
  }
  ce->raw_request = ce->raw_request ? 
    realloc(ce->raw_request, read_bytes + cpy_start + 1) :
    malloc(read_bytes + 1);
  memcpy(ce->raw_request + cpy_start, req_buf, read_bytes);
  ce->raw_request[cpy_start + read_bytes] = '\0';

  // find end of headers
  char* eoh = strstr(ce->raw_request, "\r\n\r\n");
  if (eoh == NULL)
  {
    // haven't read all the headers yet
    return -1;
  }

  // check for content-length
  if (ce->content_length == 0)
  {
    char* content_len_hdr = strstr(ce->raw_request, "Content-Length:");
    if (content_len_hdr != NULL && content_len_hdr < eoh)
    {
      ce->content_length = strtol(content_len_hdr + 15, NULL, 10);
    }
  }

  if (ce->content_length > 0)
  {
    int currlen = strlen(eoh + 4);
    if (currlen < ce->content_length)
    {
      // haven't read all the content yet
      return -1;
    }

    if (currlen > ce->content_length)
    {
      // truncate raw_request buffer according to content-length header
      int total_len = ((eoh + 4) - ce->raw_request) + (ce->content_length + 1);
      ce->raw_request = realloc(ce->raw_request, total_len);
      ce->raw_request[total_len] = '\0';
    }
  }

  return 0;
}


struct client_entry* await_active_client()
{
  int max_socket = reload_socket_set();
  struct client_entry *active_client;
  int new_socket;

  struct sockaddr addr;
  socklen_t addrlen = sizeof(addr);

  // wait for active client
  if (select(max_socket + 1, &socket_set, NULL, NULL, NULL) == -1)
  {
    log_crit("await_active_client(): select(): %s", strerror(errno));
  }

  // find out which one is active
  if ((active_client = get_active_client()) == NULL)
  {
    log_err("await_active_client(): no active socket");
    return NULL;
  }

  // if server socket is active, there is a new connection
  if (active_client == &server_entry)
  {
    // accept connection
    if ((new_socket = accept(server_entry.sock, (struct sockaddr*) &addr, &addrlen)) == -1)
    {
      log_err("await_active_client(): accept(): %s", strerror(errno));
      return NULL;
    }

    // set recv() timeout to 1s
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;

    if (setsockopt(new_socket, SOL_SOCKET, SO_RCVTIMEO, (const void*)&tv, sizeof(tv)) == -1) {
      log_err("await_active_client(): setsockopt(): %s", strerror(errno));
      close(new_socket);
      return NULL;
    }

    add_client(new_socket);
    return NULL;
  }

  if (read_client_request(active_client) == -1)
  {
    return NULL;
  }

  return active_client;
}
