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
#include <iostream>
using namespace std;
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
	// Get ip address
	char ip_v[80];
	const char* p = inet_ntop(AF_INET, &serv_addr.sin_addr, ip_v, 80);
    if(p != NULL)
    {
        std::cout << "Local IP address is: " << ip_v << std::endl;
    }
	// This is the message
	char buffer[8192];
	// This is the message serialized
	std::string message_serialized;
	// This is the protocol format of the UserRegister
	chat::UserRegister *reg = new chat::UserRegister();
	reg->set_username(argv[1]);//setusername
	reg->set_ip(ip_v);//setip
	// This is the protocol format of the UserRequest in this case 4 register
	chat::UserRequest *request = new chat::UserRequest();
	request->set_option(1); // option 1 means register
	request->set_allocated_newuser(reg); //the user register entered
	request->SerializeToString(&message_serialized); //serializetostring
	// Sending the message
	strcpy(buffer, message_serialized.c_str());
	send(client_fd, buffer, message_serialized.size()+1, 0);
	printf("Establishing connection ...\n");
	valread = read(client_fd, buffer, 8192);
	chat::ServerResponse *registerResponse = new chat::ServerResponse();
	registerResponse->ParseFromString(buffer);
	cout<<registerResponse->servermessage()<<endl;
	if(registerResponse->code()!=200){
		close(client_fd);
	}
	// This is while socket not closed
	while (true)
	{
		/* code */
	}
	

	// closing the connected socket
	close(client_fd);
	return 0;
}
