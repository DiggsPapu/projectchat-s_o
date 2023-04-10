/**
 * Diego Andres Alonzo Medinilla - 20172
 * S_O 2023 Project 1
*/
#include <iostream>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <semaphore.h>
#include "project.pb.h"
using namespace std;
sem_t semaphore_clients;
struct ChatClient
{
    int socketFd;
    std::string username;
    std::string ipAddr;
    int status;
};
// Mutex
pthread_mutex_t lock;
// Client list
std::unordered_map<std::string, ChatClient*> clients;
struct ChatClient createClient(char buffer[8192]){
	// Creating the class userequest to parse from string the buffer
	chat::UserRequest *receivedValue = new chat::UserRequest();
	receivedValue->ParseFromString(buffer);
	struct ChatClient newUser;
	newUser.username = (receivedValue->newuser().username());
	newUser.status = 1;
	newUser.ipAddr = (receivedValue->newuser().ip());
	return newUser;
}
// Print the connected clients
void printConnectedClients(){
	for (auto i:clients){
		cout<<"username: "<<i.first<<" ip: "<<i.second<<endl;
	}
}
// To check if in client list
int inClientList(struct ChatClient newUser){
	char buffer[8192] = { 0 };
	for (auto i:clients)
		{
			if(clients.find(newUser.username)!=clients.end() ) //|| i.second->ipAddr==receivedValue->newuser().ip()
			{
				// Sending a response of error
				chat::ServerResponse *response = new chat::ServerResponse();
				response->set_option(1);
				response->set_code(400);
				response->set_servermessage("Connection Error, the username/ip is already registered");
				// This is the message serialized
				std::string message_serialized;
				response->SerializeToString(&message_serialized);
				strcpy(buffer, message_serialized.c_str());
				send(newUser.socketFd, buffer, message_serialized.size()+1, 0);
				cout<<"Connection failed (ERROR 400) with the username "<<newUser.username<<" and ip "<<newUser.ipAddr<<endl;
				close(newUser.socketFd);
				return 0;
			}
		}
		clients[newUser.username]=&newUser;
		cout<<"Connection established with the username "<<newUser.username<<" and ip "<<newUser.ipAddr<<endl;
		return 1;
}
// Handling client responses and everything in thread
void *handlingClient(void *params)
{
	struct ChatClient *user = (struct ChatClient *) params;
	char buffer[8192];
	// Sending a response of connection established
	chat::ServerResponse *response = new chat::ServerResponse();
	response->set_option(1);
	response->set_code(200);
	response->set_servermessage("Connection established correctly!\n");
	// This is the message serialized
	std::string message_serialized;
	response->SerializeToString(&message_serialized);
	strcpy(buffer, message_serialized.c_str());
	send(user->socketFd, buffer, message_serialized.size()+1, 0);
	printConnectedClients();
	chat::UserRequest *request = new chat::UserRequest();
	while (user->status!=3)
	{
		cout<<user->username<<" socket: "<<user->socketFd<<endl;
		read(user->socketFd, buffer, 8192);	
		request->ParseFromString(buffer);
		switch (request->option())
		{
			case 0:
			{
				break;
			}
			case 3:
			{
				switch (request->status().newstatus())
				{
					case 1:
					{
						cout<<"Client "<<user->username<<" with the ip "<<user->ipAddr<<" has changed status to ACTIVE"<<endl;
						user->status = 1;
						break;
					}
					case 2:
					{
						cout<<"Client "<<user->username<<" with the ip "<<user->ipAddr<<" has changed status to OCCUPATED"<<endl;
						user->status = 2;
						break;
					}
					case 3:
					{
						cout<<"Client "<<user->username<<" with the ip "<<user->ipAddr<<" has changed status to INACTIVE, logged out"<<endl;
						user->status = 3;
						clients.erase(user->username);
						close(user->socketFd);
						break;
					}					
					default:{cout<<"Client "<<user->username<<" with the ip "<<user->ipAddr<<" has sent an invalid status"<<endl;break;}
				}
				break;
			}
			default:
			{
				cout<<"Client "<<user->username<<" with the ip "<<user->ipAddr<<" sent an invalid value"<<endl;
				break;
			}			
		}
	}
	cout<<"Connection ended with the username "<<user->username<<" and ip "<<user->ipAddr<<endl;
	printConnectedClients();
    pthread_exit(0);
}
int main(int argc, char const* argv[])
{
	// Semaphore shared between threads init
	sem_init(&semaphore_clients, 0, 1);
    // In case the port is not indicated
    if (argc != 2)
    {
        fprintf(stderr, "Use: server <serverport>\n");
        return 1;
    }
	// Assigning port
	int port = atoi(argv[1]);
    /**
     * CREATING THE SOCKET AND CONNFIGURING IT
    */
	int server_fd, new_socket;
	struct sockaddr_in address, new_connection;
	struct sockaddr_storage serverStorage;
	int opt = 1;
	socklen_t addrlen = sizeof(serverStorage);
	char buffer[8192] = { 0 };

	// Creating socket file descriptor
	if ((server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		perror("Server: socket failed");
		exit(EXIT_FAILURE);
	}

	// Forcefully attaching socket to the port any
	if (setsockopt(server_fd, SOL_SOCKET,SO_REUSEADDR | SO_REUSEPORT, &opt,sizeof(opt))) {
		perror("Server: set socket options failed");
		exit(EXIT_FAILURE);
	}
	address.sin_family = AF_INET;
    // // Aws ip public should be defined here:
    // serv_addr.sin_addr.s_addr = inet_addr("dirección IP pública de la instancia de AWS");
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);

	// Forcefully attaching socket to the port entered
	if (bind(server_fd, (struct sockaddr*)&address,sizeof(address))< 0) {
		perror("Server: bind failed IP socket->port");
		exit(EXIT_FAILURE);
	}
	if (listen(server_fd, 5) != 0) { //5 first connection before refused.
		perror("listen\n");
		exit(EXIT_FAILURE);
	}
	printf("Listening on port %d\n", port);\
	while (true){
		if ((new_socket= accept(server_fd, (struct sockaddr*)&serverStorage,(socklen_t*)&addrlen))< 0) {
		perror("accept");
		exit(EXIT_FAILURE);
		}
		// ____________CONNECTION_____________
		// Got the response and saved it in buffer
		recv(new_socket, &buffer, sizeof(buffer),0);
		// Create a new user with a function and assigning sockets
		struct ChatClient newUser = createClient(buffer);
		newUser.socketFd = new_socket;
		pthread_t thread_id;
		if(inClientList(newUser)){
			pthread_create(&thread_id,NULL, handlingClient, (void *)&newUser);
		}
	}
	// closing the connected socket
	close(new_socket);
	// closing the listening socket
	shutdown(server_fd, SHUT_RDWR);
	return 0;
}
