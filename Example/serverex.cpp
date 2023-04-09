/**
 * Diego Andres Alonzo Medinilla - 20172
 * S_O 2023 Project 1
*/
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include "project.pb.h"

int main(int argc, char const* argv[])
{
    // In case the port is not indicated
    if (argc != 2)
    {
        fprintf(stderr, "Use: server <puertodelservidor>\n");
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
	char buffer[1024] = { 0 };
	char* hello = "Hello from server";

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
	if ((new_socket= accept(server_fd, (struct sockaddr*)&address,(socklen_t*)&addrlen))< 0) {
		perror("accept");
		exit(EXIT_FAILURE);
	}
	printf("Listening on port %d\n", port);
	pthread_t tid[5];
	int i = 0;
	while (1)
	{
		new_conn_size = sizeof new_connection;
		 if (accept(server_fd, (struct sockaddr *)&new_connection, &new_conn_size)==-1)
		 {
			perror("Error in accept");
            continue;
		 }
	}
	
	valread = read(new_socket, buffer, 1024);
	printf("%s\n", buffer);
	send(new_socket, hello, strlen(hello), 0);
	printf("Hello message sent\n");

	// closing the connected socket
	close(new_socket);
	// closing the listening socket
	shutdown(server_fd, SHUT_RDWR);
	return 0;
}
