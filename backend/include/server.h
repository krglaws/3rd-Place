#ifndef SERVER_H
#define SERVER_H

#include <kylestructs.h>

/* struct in6_addr definition */
#include <arpa/inet.h>

/* Server settings */
#define DEFAULT_PORT (80)
#define DEFAULT_ADDR in6addr_loopback
#define DEFAULT_MAX_CLIENTS (100)

#define HTTPVERSION "HTTP/1.1"
#define HTTPVERSION_ALT "HTTP/1.0"

#define MAXREQUESTSIZE (10240)

/* struct response definition */
#include <response.h>

/* struct auth_token definition */
#include <auth_manager.h>

/* Prints usage string */
void usage(const char* name);


/* Commandline options container */
struct options
{
  struct in6_addr server_addr;
  uint16_t server_port;
  int max_clients;
  char* logpath;
};


/* struct containing stripped down request info */
struct request
{
  /* if client is logged in, this will
     contain the clients user name and
     user ID */
  struct auth_token* client_info;

  /* will accept either 1.1 or 1.0 */
  char* http_version;

  /* POST, GET, PUT, DELETE, or HEAD */
  char* method;

  /* path to requested resource */
  char* uri;

  /* if a query string is present,
     this will contain the key-value
     pairs */
  ks_hashmap* query;

  /* key-value pair for each header */
  ks_hashmap* header;

  /* cookies parsed from header */
  ks_hashmap* cookies;

  /* key-value pair for each parameter
     in request content */
  ks_hashmap* content;  
};

#endif
