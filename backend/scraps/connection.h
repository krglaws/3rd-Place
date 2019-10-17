
#ifndef _CONNECTION_H_
#define _CONNECTION_H_

#define CHILD_STACK_SIZE (1024 * 1024)

#define BUFFER_SIZE (1024)


struct thread
{
  pid_t pid;
  
  int sock_fd;
  struct sockaddr_in* addr;

  uint8_t* stack;
  bool is_dead;

  struct thread* next;
  struct thread* prev;
};


struct thread_list
{
  int num_threads;
  struct thread* head;
};


/* Called by main thread to set up new child thread for
   handling requests on that connection */
int new_connection(int fd, struct sockaddr_in* address);


/* Called when main thread sends SIGINT signal to thread */
static void handle_sigint(int signum);


/* child thread's 'main' function. Just continuously loops, listening
   for requests until the connection is closed by the client */
int serve_connection(struct thread* t);


/* Just prints request header and body */
int evaluate(int socket_fd, struct sockaddr_in* address, size_t bytes, char* buffer);


/* When a new connection is created, this is used 
   to add the new thread's information to the global list */
void add_to_list(struct thread* t);


/* Called by main thread whenever SIGCHLD is received */
void manage_threads();


/* Called by manage_threads(). Removes dead thread info
   from global list */
int cleanup_thread(struct thread* t);


#endif

