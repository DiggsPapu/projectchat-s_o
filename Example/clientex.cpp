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
	if (sa->sa_family == AF_INET)
	{
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
			cout << "Error: "<< serverMsg.servermessage()<<endl;
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

		if (connected == 0)
		{
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

	if (argc != 4)
	{
		fprintf(stderr, "Use: client <username> <server ip> <server port>\n");
		exit(1);
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(argv[2], argv[3], &hints, &servinfo)) != 0)
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// Conectarse a la opcion que este disponible
	for (p = servinfo; p != NULL; p = p->ai_next)
	{
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
							 p->ai_protocol)) == -1)
		{
			perror("ERROR: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1)
		{
			perror("ERROR: connect client");
			close(sockfd);
			continue;
		}

		break;
	}
	// Indicar fallo al conectarse
	if (p == NULL)
	{
		fprintf(stderr, "ERROR: failed to connect\n");
		return 2;
	}

	//Completar la coneccion
	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
			  s, sizeof s);
	printf("CONNECTED IP: %s\n", s);
	freeaddrinfo(servinfo);
    // Message register
	char buffer[8192];
	std::string message_serialized;
    chat::UserRequest *request = new chat::UserRequest();
    chat::UserRegister *reg = new chat::UserRegister();
    reg->set_username(argv[1]);
    reg->set_ip(s);
    request->set_option(1);
    request->set_allocated_newuser(reg);
    request->SerializeToString(&message_serialized);
	strcpy(buffer, message_serialized.c_str());
	send(sockfd, buffer, message_serialized.size() + 1, 0);
    // Obtener response de servidor
	recv(sockfd, buffer, 8192, 0);
    chat::ServerResponse serverMessage;
	serverMessage.ParseFromString(buffer);
    // En caso de registro no exitoso
	if(serverMessage.code() != 200){
			std::cout << serverMessage.servermessage()<< std::endl;
			return 0;
	}
    // En caso de registro exitoso
	std::cout << "SERVER: "<< serverMessage.servermessage()<< std::endl;	
	connected = 1;
    // despachar thread que escucha mensajes del server
	pthread_t thread_id;
	pthread_attr_t attrs;
	pthread_attr_init(&attrs);
	pthread_create(&thread_id, &attrs, listenToMessages, (void *)&sockfd);
	int client_opt, proceed = 1;
    while (proceed)
    {
        printMenu();
        cin>>client_opt;
        while (waitingForServerResponse == 1){}
        switch (client_opt)
        {
            case 7:{
                int option;
                printf("Log out\n1. y\n2. n\n");
                std::cin >> option;
                switch (option)
                {
                    case 1:{
                        printf("Succesfully logged out\n");
                        proceed = 0;
                        break;
                    }
                    case 2:{
                        break;
                    }
                    default:{
                        printf("The value entered is invalid\n");
                        break;
                    }
                }
                break;
            }
            default:{
                break;
            }
        }
    }
    
	return 0;
}