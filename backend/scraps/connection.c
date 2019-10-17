
#define _GNU_SOURCE
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sched.h>
#include <signal.h>

#include "connection.h"


/* Global list of active threads */
struct thread_list tlist = { 0, NULL };


int new_connection(int fd, struct sockaddr_in* address)
{
  /* gotta make new copy of address, since it will be wiped in main() */
  struct sockaddr_in* address_cpy = malloc(sizeof(struct sockaddr_in));
  memcpy(address_cpy, address, sizeof(struct sockaddr_in));
  
  struct thread* t = calloc(1, sizeof(struct thread));
  t->stack = malloc(CHILD_STACK_SIZE);
  t->addr = address_cpy;
  t->sock_fd = fd;

  /* add to thread to list prior to starting to ensure thread 
     can only die after it is on the queue, not before */
  add_to_list(t);

  pid_t pid;
  if ((pid = clone(serve_connection, t->stack + CHILD_STACK_SIZE, CLONE_FILES | SIGCHLD, (void*)t)) == -1)
  {
    perror("Failed to create new thread");
    cleanup_thread(t);
  }
  else t->pid = pid;

  return pid;
}


static void handle_sigint(int signum)
{
  pid_t pid = getpid();

  struct thread* t = tlist.head;
  while (t->pid != pid) t = t->next;
  t->is_dead = 1;
  close(t->sock_fd);

  printf("Stopping thread %d... \n", pid);

  exit(EXIT_SUCCESS);
}


int serve_connection(struct thread* t)
{
  struct sockaddr_in* addr = t->addr;
  int sock_fd = t->sock_fd;

  int err = 0;
  int in_bytes = 0;
  char in_buffer[BUFFER_SIZE] = {0};
  char ip_str[20] = {0};

  if (inet_ntop(addr->sin_family, &addr->sin_addr, ip_str, sizeof(ip_str)) == NULL)
  {
    perror("inet_ntop");
    printf("Connection closed\n");
    t->is_dead = true;
    return -1;
  }
  else printf("Connected to %s\n", ip_str);

  while(1)
  {
    if ((in_bytes = recv(sock_fd, in_buffer, BUFFER_SIZE, 0)) < 0)
      break;

    if (in_bytes > 0)
    {
      evaluate(sock_fd, addr, in_bytes, in_buffer);
      memset(in_buffer, 0, BUFFER_SIZE);
    }
  }

  close(sock_fd);

  if (inet_ntop(addr->sin_family, &addr->sin_addr, ip_str, sizeof(ip_str)) == NULL)
  {
    perror("inet_ntop");
    printf("Connection closed\n");
  }
  else printf("Connection with %s closed.\n", ip_str);

  t->is_dead = true;
  return err;
}


int evaluate(int socket_fd, struct sockaddr_in* address, size_t bytes, char* buffer)
{
  printf("Message from client:\n%s", buffer);
  return 0;
}


void add_to_list(struct thread* t)
{
  struct thread* head = tlist.head;
  if (head == NULL) 
  {
    tlist.head = t;
    return;
  }
  while (head->next) head = head->next;
  head->next = t;
  t->prev = head;
}


void manage_threads()
{
  struct thread* t = tlist.head;
  while (t != NULL)
  {
    if (t->is_dead) {cleanup_thread(t); return;}
    else t = t->next;
  }
}


int cleanup_thread(struct thread* t)
{
  if (t == NULL) return -1;
  if (t->next && t->prev)
  {
    t->prev->next = t->next;
    t->next->prev = t->prev;
    free(t->stack);
    free(t);
    return 0;
  }
  else if (t->next)
  {
    tlist.head = t->next;
    t->next->prev = NULL;
    free(t->stack);
    free(t);
    return 0;
  }
  else if (t->prev)
  {
    t->prev->next = NULL;
    free(t->stack);
    free(t);
    return 0;
  }
  else
  {
    tlist.head = NULL;
    free(t->stack);
    free(t);
    return 0;
  }
}


