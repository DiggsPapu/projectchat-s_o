#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include "project.pb.h"

#define MAX_MESSAGE_LENGTH 1024

int main(int argc, char *argv[])
{
    if (argc > 1)
    {
        // File descriptor 4 the sockets identity, the port is passed as a parameter
        int fd,fd2,client_len,port;
        // The port is extracted and stored in the port 
        port = atoi(argv[1]);
        // Socket address
        // Socket store bout server and socket of client
        struct sockaddr_in server;
        struct sockaddr_in client;
        //___Server config____
        // Afinet defines for tcp/ip
        server.sin_family = AF_INET; 

	printf("\nCREATED\n");
    }
    else
    {
        printf("The port wasn't entered.\n");
        return 0;
    }
    return 0;
}
