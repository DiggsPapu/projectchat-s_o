// Client side C/C++ program to demonstrate Socket
// programming
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include "project.pb.h"
#define PORT 8080

int main(int argc, char const* argv[])
{

	printf("%s\n",argv[1]);
	int status, valread, client_fd;
	struct sockaddr_in serv_addr;
	char* hello = "Hello from client";
	if (argc != 4)
	{
		fprintf(stderr, "Client use: client <username> <server_ip> <server_port>\n");
		exit(1);
	}
	if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("\n Socket creation error \n");
		return -1;
	}
    int port = atoi(argv[3]);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);

	// Convert IPv4 and IPv6 addresses from text to binary
	// form
	if (inet_pton(AF_INET, argv[2], &serv_addr.sin_addr)
		<= 0) {
		printf(
			"\nInvalid address/ Address not supported \n");
		return -1;
	}

	if ((status = connect(client_fd, (struct sockaddr*)&serv_addr,sizeof(serv_addr)))< 0) {
		printf("\nConnection Failed \n");
		return -1;
	}
	// This is the message
	char buffer[8192];
	// send(client_fd, hello, strlen(hello), 0);
    // printf("Hello message sent\n");
    // valread = read(client_fd, buffer, 1024);
    // printf("%s\n", buffer);
	// This is the message serialized
	std::string message_serialized;
	// This is the protocol format of the user info
	chat::UserInfo *reg = new chat::UserInfo();
	reg->set_username(argv[1]);//setusername
	reg->set_ip(argv[2]);//setip
	reg->set_status(1); //setstatus as active in the begginning
	reg->SerializeToString(&message_serialized); //serializetostring
	// Sending the message
	strcpy(buffer, message_serialized.c_str());
	send(client_fd, buffer, message_serialized.size()+1, 0);
	printf("Establishing connection ...\n");
	valread = read(client_fd, buffer, 1024);
	printf("%s\n", buffer);

	// closing the connected socket
	close(client_fd);
	return 0;
}
