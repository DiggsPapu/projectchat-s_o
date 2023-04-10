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
// Print menu
void printMenu(){
	cout<<"1 -> Chat with everyone in the chat (Broadcasting)"<<endl;
	cout<<"2 -> Send a private message"<<endl;
	cout<<"3 -> Change status"<<endl;
	cout<<"4 -> List connected users in the chat system"<<endl;
	cout<<"5 -> Deploy info from a particular user"<<endl;
	cout<<"6 -> Help"<<endl;
	cout<<"7 -> Exit"<<endl;
}
int main(int argc, char const* argv[])
{
	printf("%s\n",argv[1]);
	int status, valread, client_fd;
	struct sockaddr_in serv_addr;
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

	// Convert IPv4 and IPv6 addresses from text to binary form
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
	// Reading a response from server
	valread = read(client_fd, buffer, 8192);
	// Server response protocol format
	chat::ServerResponse *registerResponse = new chat::ServerResponse();
	registerResponse->ParseFromString(buffer);
	cout<<registerResponse->servermessage()<<endl;
	// In case this connection is not good the socket is closed and exit from the program
	if(registerResponse->code()!=200){
		close(client_fd);
		return 0;
	}
	
	// This is while socket not closed
	int option,open = 1;
	while (open)
	{
		printMenu();
		cin>>option;
		switch (option)
		{
			case 7:
			{
				chat::ChangeStatus *status = new chat::ChangeStatus();
				status->set_newstatus(3);
				status->set_username(argv[1]);
				chat::UserRequest *ureq = new chat::UserRequest();
				ureq->set_option(3); // option 1 means register
				ureq->set_allocated_status(status); //the user register entered
				ureq->SerializeToString(&message_serialized); //serializetostring
				// Sending the message
				strcpy(buffer, message_serialized.c_str());
				send(client_fd, buffer, message_serialized.size()+1, 0);
				close(client_fd);
				printf("Logging out.\n");
				open = 0;
				break;
			}
			default:
			{
				cout<<"The option entered is invalid"<<endl;
				break;
			}
		}
	}
	// closing the connected socket
	close(client_fd);
	return 0;
}