
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>


/* Server settings */
#define LISTENQUEUE 	(10)
#define BUFFERSIZE 	(1024)
#define SERVERADDR 	(0x00000000)
#define SERVERPORT 	(80)

/* Request types */
#define HTTP_POST 	(0)
#define HTTP_GET     	(1)
#define HTTP_PUT     	(2)
#define HTTP_DELETE  	(3)
#define HTTP_EMPTY 	(4)
#define HTTP_UNKOWN	(5)


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


int check_req_type(char* buffer, int bufflen)
{
	if (buffer == NULL || bufflen < 1) return HTTP_EMPTY;
	char type_str[10] = {0};
	sscanf(buffer, "%s", type_str);

	/* Create */
	if (strcmp(type_str, "POST") == 0) return HTTP_POST;

	/* Read */
	if (strcmp(type_str, "GET") == 0) return HTTP_GET;

	/* Update */
	if (strcmp(type_str, "PUT") == 0) return HTTP_PUT;
	
	/* Delete */
	if (strcmp(type_str, "DELETE") == 0) return HTTP_DELETE;

	return HTTP_UNKOWN;
}


void handle_post(int client_sock, char* buffer, int bufflen)
{
	char path_str[100] = {0};
	
	sscanf(buffer, "%s", "%s", NULL, path_str); 

	printf("POST to %s\n", path_str);

	send(client_sock, NULL, 0, 0);
}


void handle_get(int client_sock, char* buffer, int bufflen)
{
	char path_str[100] = {0};
	
	sscanf(buffer, "%s", "%s", NULL, path_str); 

	printf("GET %s\n", path_str);
	
	send(client_sock, NULL, 0, 0);
}


void handle_put(int client_sock, char* buffer, int bufflen)
{	char path_str[100] = {0};
	
	sscanf(buffer, "%s", "%s", NULL, path_str); 

	printf("PUT to %s\n", path_str);	send(client_sock, NULL, 0, 0);
}


void handle_delete(int client_sock, char* buffer, int bufflen)
{	
	char path_str[100] = {0};
	
	sscanf(buffer, "%s", "%s", NULL, path_str); 

	printf("DELETE %s\n", path_str);
	
	send(client_sock, NULL, 0, 0);
}


void evaluate_request(int client_sock, char* buffer, int bufflen)
{
	int request_type = check_req_type(buffer, bufflen);

	int out_bytes = 0;

	char resp_501[] = "HTTP/1.1 501 Not Implemented\nContent-Type: text/html\nContent-Length: 0\n\n";

	switch (request_type)
	{
		case HTTP_POST:
			handle_post(client_sock, buffer, bufflen);
			break;
		case HTTP_GET:
			handle_get(client_sock, buffer, bufflen);
			break;
		case HTTP_PUT:
			handle_put(client_sock, buffer, bufflen);
			break;
		case HTTP_DELETE:
			handle_delete(client_sock, buffer, bufflen);
			break;
		case HTTP_EMPTY:
			printf("Empty request, do nothing.\n");
			break;
		case HTTP_UNKNOWN:
			out_bytes = send(client_sock, resp_501, sizeof(resp_501), 0);
			printf("501: unkown request. Sent %d bytes.\n", out_bytes);
			break;
	}
}


