
char* receive_message(int client_fd, struct sockaddr_in* client_addr)
{
  char* message_buffer = calloc(1, BUFFERLEN);
  char* curr_buff_ptr = message_buffer;
  int multiplier = 1;
  int bytes = 0, cont_bytes = 0;
  
  while (multiplier++ < MAXBUFFERMUL+1)
  {
    if (multiplier >= MAXBUFFERMUL)
    {
      fprintf(stderr, "Request exceeds %d limit\n", MAXBUFFERMUL * BUFFERLEN);
      free(message_buffer);
      return NULL;
    }
    if ((bytes = recv(client_fd, curr_buff_ptr, BUFFERLEN-1, 0)) == -1)
    {
      perror("receive_message(): error while reading message from client");
      exit(EXIT_FAILURE);
    }
    if (bytes == 0) break;
    
    message_buffer = realloc(message_buffer, BUFFERLEN * multiplier);
    curr_buff_ptr = message_buffer + strlen(message_buffer);
    memset(curr_buff_ptr, 0, BUFFERLEN);
  }

  if (!strstr(message_buffer, "\r\n\r\n"))
  {
    printf("Header missing empty line\n");
    free(message_buffer);
    return NULL;
  }

  char* contlen = "Content-Length: ";
  contlen = strstr(message_buffer, contlen);
  contlen && sscanf(message_buffer, "%*s %d", &cont_bytes);

  if (cont_bytes != strlen(contlen))
  {
    printf("receive_message(): Content-Length mismatch: header says content length is %d bytes, but received %d bytes\n", cont_bytes, strlen(contlen));
    free(message_buffer);
    return NULL;
  }

  return message_buffer;
}
