
char* receive_message()//int client_fd, struct sockaddr_in* client_addr)
{
  char* message_buffer = calloc(1, BUFFERLEN);
  char* curr_buff_ptr = message_buffer;
  int curr_buff_len = BUFFERLEN;
  int max_tries = -10;
  int bytes = total_bytes = head_bytes = cont_bytes = 0;
  
  while (max_tries--)
  {
    
  }
  return NULL;
}
