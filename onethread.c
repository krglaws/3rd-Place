
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>


#define LISTENQUEUE (10)
#define BUFFERSIZE (1024)
#define SERVERADDR (0x00000000)
#define SERVERPORT (80)


int main()
{
	int server_sock, out_bytes;
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	char out_buffer[BUFFERSIZE] = {0};
	
        int client_sock, client_addr_size, in_bytes;
	struct sockaddr_in client_addr;
	memset(&client_addr, 0, sizeof(client_addr));
	char in_buffer[BUFFERSIZE] = {0};

	char ip_str[30] = {0};

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVERPORT);
	server_addr.sin_addr.s_addr = htonl(SERVERADDR);

	if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("Failed to create server socket");
		exit(EXIT_FAILURE);
	}

	if ((bind(server_sock, (struct sockaddr *) &server_addr, sizeof(server_addr))) == -1)
	{
		perror("Failed to bind server socket to address");
		exit(EXIT_FAILURE);
	}

	if (listen(server_sock, LISTENQUEUE) == -1)
	{
		perror("Failed to designate server socket");
		exit(EXIT_FAILURE);
	}

	while(1)
	{
		printf("Wating for new connections... \n");
		client_addr_size = sizeof(struct sockaddr_in);
		if ((client_sock = accept(server_sock, (struct sockaddr *) &client_addr, &client_addr_size)) == -1)
		{
			perror("Error while awaiting connections");
			exit(EXIT_FAILURE);
		}
		
		if (inet_ntop(client_addr.sin_family, &(client_addr.sin_addr), ip_str, sizeof(ip_str)) == NULL)
		{
			perror("Failed read client address");
			exit(EXIT_FAILURE);
		}
		else
		{
			printf("Connected to %s.\n", ip_str);
		}

		if ((in_bytes = recv(client_sock, in_buffer, BUFFERSIZE, 0)) == -1)
		{
			perror("Error while reading message from client");
			exit(EXIT_FAILURE);
		}

		if (in_bytes > 0)
		{		
			printf("Message from %s:\n\n%s\n", ip_str, in_buffer);
		}
		
		printf("Sending response to %s... ", ip_str);
		char response[] = "HTTP/1.1 200 OK\nContent-Type:text/html\nContent-Length:12\n\nHello there!";
		if ((out_bytes = send(client_sock, response, sizeof(response), 0)) == -1)
		{
			perror("Failed to send response");
			exit(EXIT_FAILURE);
		}
		printf("%d bytes sent.\n", out_bytes);

		// cleanup
		close(client_sock);
		printf("Connection to %s closed.\n", ip_str);
		memset(&client_addr, 0, sizeof(client_addr));
		memset(ip_str, 0, sizeof(ip_str));
		memset(in_buffer, 0, sizeof(in_buffer));
	}
}

void evaluate_request(int client_sock, char* buffer, int buflen)
{
	
}
