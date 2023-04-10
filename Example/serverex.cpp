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
struct UserData
{
	int u_socket,status;
	std::string username;
    char ipAddr[INET_ADDRSTRLEN]; // INET_ADDRSTRLEN defines the max length of characters of an IP address
};
// Client list
std::unordered_map<std::string, chat::UserInfo*> clients;
void *Register(void *params)
{
	if (clients.size())
	{
		/* code */
	}
	while (true)
	{
		/* code */
	}
	
}
int main(int argc, char const* argv[])
{
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
	int server_fd, new_socket, valread;
	struct sockaddr_in address, new_connection;
    socklen_t new_conn_size;
	int opt = 1;
	int addrlen = sizeof(address);
	char buffer[8192] = { 0 };

	// Creating socket file descriptor
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
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
	if (listen(server_fd, 5) < 0) { //5 first connection before refused.
		perror("listen");
		exit(EXIT_FAILURE);
	}
	printf("Listening on port %d\n", port);
	while (true)
	{
		if ((new_socket= accept(server_fd, (struct sockaddr*)&address,(socklen_t*)&addrlen))< 0) 
		{
		perror("accept");
		exit(EXIT_FAILURE);
		}
		/**
		 * RECEPTION
		*/
		// Got the response and saved it in buffer
		valread = read(new_socket, buffer, 8192);
		// Creating the class userequest to parse from string the buffer
		chat::UserRequest *receivedValue = new chat::UserRequest();
		receivedValue->ParseFromString(buffer);
		printf("The option gotten was: %d\n",receivedValue->option());
		// Create a new user and send it to a function to know if it already exists
		chat::UserInfo *newUser = new chat::UserInfo();
		newUser->set_ip(receivedValue->newuser().ip());
		newUser->set_username(receivedValue->newuser().username());
		newUser->set_status(1);
		for (auto i:clients)
		{
			if(clients.find(newUser->username())==clients.end() || i.second->ip()==newUser->ip())
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
				send(new_socket, buffer, message_serialized.size()+1, 0);
				cout<<"Connection failed (ERROR 400) with the username "<<newUser->username()<<" and ip "<<newUser->ip()<<endl;
			}
		}
		cout<<"Connection established with the username "<<newUser->username()<<" and ip "<<newUser->ip()<<endl;
		// Sending a response of received
		chat::ServerResponse *response = new chat::ServerResponse();
		response->set_option(1);
		response->set_code(200);
		response->set_servermessage("Connection established correctly!\n");
		// This is the message serialized
		std::string message_serialized;
		response->SerializeToString(&message_serialized);
		strcpy(buffer, message_serialized.c_str());
		send(new_socket, buffer, message_serialized.size()+1, 0);
		
		pthread_t thread_id;
        pthread_attr_t attrs;
        pthread_attr_init(&attrs);
        // pthread_create(&thread_id, &attrs, Register, (void *)&newClient);
	}
	// closing the connected socket
	close(new_socket);
	// closing the listening socket
	shutdown(server_fd, SHUT_RDWR);
	return 0;
}
