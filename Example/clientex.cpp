// Client side C/C++ program to demonstrate Socket
// programming
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include "project.pb.h"
#include <iostream>
using namespace std;

int connected, waitingForServerResponse, waitingForInput;
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
// get sockaddr
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET){
		return &(((struct sockaddr_in *)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}
void *listenToMessages(void *args)
{
	while (1)
	{
		char bufferMsg[8192];
		int *sockmsg = (int *)args;
		chat::ServerResponse serverMsg;
		int bytesReceived = recv(*sockmsg, bufferMsg, 8192, 0);
		serverMsg.ParseFromString(bufferMsg);
		if (serverMsg.code() != 200)
		{
			printf("________________________________________________________\n");
			cout << "ERROR: "<< serverMsg.servermessage()<<endl;
		}
		else if (serverMsg.code() == 200 || serverMsg.option()==4)
		{
            chat::newMessage message = serverMsg.message();
			printf("________________________________________________________\n");
			std::cout <<message.sender()<<": \n"<<message.message()<< std::endl;
		}
		else
		{
			printf("ERROR: the server sent an invalid value\n");
			break;
		}
		waitingForServerResponse = 0;
		if (connected == 0){
			pthread_exit(0);
		}
	}
}
int main(int argc, char const* argv[])
{
    // Estructura de la coneccion
	int sockfd, numbytes;
	char buf[8192];
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];
	if (argc != 4){
		fprintf(stderr, "Use: client <username> <server ip> <server port>\n");
		exit(1);
	}
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	if ((rv = getaddrinfo(argv[2], argv[3], &hints, &servinfo)) != 0){
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}
	for (p = servinfo; p != NULL; p = p->ai_next)
	{
		if ((sockfd = socket(p->ai_family, p->ai_socktype,p->ai_protocol)) == -1){
			perror("ERROR: socket");
			continue;
		}
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1){
			perror("ERROR: connect client");
			close(sockfd);
			continue;
		}
		break;
	}
	if (p == NULL){
		fprintf(stderr, "ERROR: failed to connect\n");
		return 2;
	}
	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),s, sizeof s);
	printf("CONNECTED IP: %s\n", s);
	freeaddrinfo(servinfo);
	// Defining the structs to send messages using the protocol
	std::string message_serialized;
    chat::UserRequest *request = new chat::UserRequest();
    chat::UserRegister *reg = new chat::UserRegister();
	chat::UserInfoRequest *info = new chat::UserInfoRequest();
	chat::AllConnectedUsers *a_users = new chat::AllConnectedUsers();
	chat::newMessage *m_new = new chat::newMessage();
	chat::ChangeStatus *newStatus = new chat::ChangeStatus();
    chat::ServerResponse *serverMessage = new chat::ServerResponse();
    // Message register
	char buffer[8192];
    reg->set_username(argv[1]);
    reg->set_ip(s);
    request->set_option(1);
    request->set_allocated_newuser(reg);
    request->SerializeToString(&message_serialized);
	strcpy(buffer, message_serialized.c_str());
	send(sockfd, buffer, message_serialized.size() + 1, 0);
	recv(sockfd, buffer, 8192, 0);
	serverMessage->ParseFromString(buffer);
	if(serverMessage->code() != 200){
			std::cout << serverMessage->servermessage()<< std::endl;
			return 0;
	}
	std::cout << "SERVER: "<< serverMessage->servermessage()<< std::endl;	
	connected = 1;
	pthread_t thread_id;
	pthread_attr_t attrs;
	pthread_attr_init(&attrs);
	pthread_create(&thread_id, &attrs, listenToMessages, (void *)&sockfd);
	int client_opt, proceed = 1;
    while (proceed){
        request->Clear();
        printMenu();
        cin>>client_opt;
        while (waitingForServerResponse == 1){}
        switch (client_opt){
            case 1:{
                break;
            }
            case 4:{

                break;
            }
            case 7:{
				proceed = 0;
                break;
            }
            default:{
                break;
            }
        }
    }
	printf("Thanks for using this server chat %s\nSee ya soon!",argv[1]);
	return 0;
}