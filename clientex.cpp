/**
 * Diego Andres Alonzo Medinilla - 20172
 * Roberto Rios - 20979
 * S_O 2023 Project 1
 * Chat 
 *
 * Esta es la implementacion del menu y las funciones del cliente para comunicarse al servidor
 *
 */


#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include "proyect.pb.h"
#include <iostream>

using namespace std;

// inicializar variables globales
int connected, waitingForServerResponse, waitingForInput;

// ----------------------------------------- logica para las funciones del controlador, enviar y recibir info del server -----------------------------------------
// funcion para enviar a un thread y obtener el ip de un usuario 
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET){
		return &(((struct sockaddr_in *)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}


// este es como un event listener que dependiendo del input del usuario preguntara por una u otra funcion
void *listenToMessages(void *args)
{	
	// siempre esta escuchando y espera por una opcion que ingrese el usuario
	while (1){
		
		// inicializamos las variables para enviar el mensaje del usuario 
		char bufferMsg[8192];
		int *sockmsg = (int *)args;
		chat::ServerResponse *serverMsg = new chat::ServerResponse();
		int bytesReceived = recv(*sockmsg, bufferMsg, 8192, 0);
		serverMsg->ParseFromString(bufferMsg);
		
		// si no nos logramos conectar al servidor, se devuelve un error 
		if (serverMsg->code() != 200){
			cout <<serverMsg->servermessage()<<endl;
			
		} else {
			// para enviar mensajes, segun el protocolo, se utiliza el server response, el server message, y la ip 
			switch (serverMsg->option())
			{
			case 2:{
				std::cout<<serverMsg->servermessage();
				if(serverMsg->has_userinforesponse()){
					std::cout<<"\nUsername->"<<serverMsg->userinforesponse().username()<<"		IP->"<<serverMsg->userinforesponse().ip()<<"		Status->"<<serverMsg->userinforesponse().status()<<std::endl;
				}
				else{
					for(int i = 0; i<serverMsg->connectedusers().connectedusers_size(); i++){
						std::cout<<"\nUsername->"<<serverMsg->connectedusers().connectedusers(i).username()<<"		IP->"<<serverMsg->connectedusers().connectedusers(i).ip()<<"		Status->"<<serverMsg->connectedusers().connectedusers(i).status()<<std::endl;
					}
				}
				break;
			}
			case 3:{
				std::cout<<serverMsg->servermessage()<<std::endl;
				break;
			}
			case 4:{
				std::cout<<serverMsg->servermessage();
				std::cout<<serverMsg->message().message()<<std::endl;
			}
			default:
				break;
			}
		}
		// si se desconecta del server 
		serverMsg->Clear();
		waitingForServerResponse = 0;
		if (connected == 0){
			pthread_exit(0);
		}
	}
}


// ----------------------------------------- inicio main del cliente -----------------------------------------
// main con opciones para que ingrese el usuario
int main(int argc, char const* argv[])
{
	
	// ----------------------------------------- funciones y variables para el controlador del cliente -----------------------------------------
	// variables para enviar info al server
	int sockfd, numbytes;
	char buf[8192];
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];
	
	// si no se ingresan los params necesarios
	if (argc != 4){
		fprintf(stderr, "Use: client <username> <server ip> <server port>\n");
		exit(1);
	}
	
	// mas variables para configurar el server 
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	
	// si no se recibe la informacion del servidor hay error
	if ((rv = getaddrinfo(argv[2], argv[3], &hints, &servinfo)) != 0){
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}
	
	// buscar en los sockets si hay errores
	for (p = servinfo; p != NULL; p = p->ai_next)
	{
		// aqui se encuentra si hay error
		if ((sockfd = socket(p->ai_family, p->ai_socktype,p->ai_protocol)) == -1) {
			perror("ERROR: socket");
			continue;
		}
		
		// aqui se encuentra si hay error
        	if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			perror("ERROR: connect client");
			close(sockfd);
			continue;
		}
		break;
	}
	
	// no se encontro un socket
	if (p == NULL) {
		fprintf(stderr, "ERROR: failed to connect\n");
		return 2;
	}
	
	// se si encontro el socket
	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),s, sizeof s);
	printf("CONNECTED IP: %s\n", s);
	freeaddrinfo(servinfo);
	
	// Defining the structs to send messages using the protocol
	std::string message_serialized;
        chat::UserRequest *request = new chat::UserRequest();
        chat::UserRegister *reg = new chat::UserRegister();
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
	
	// si hubo error al buscar 
	if (serverMessage->code() != 200) {
		std::cout << serverMessage->servermessage()<< std::endl;
		return 0;
	}
	
	// si se superaron los errores, entonces se puede abrir un fork y hostear al cliente ahi
	std::cout << "SERVER: "<< serverMessage->servermessage()<< std::endl;	
	connected = 1;
	pthread_t thread_id;
	pthread_attr_t attrs;
	pthread_attr_init(&attrs);
	pthread_create(&thread_id, &attrs, listenToMessages, (void *)&sockfd);
	int proceed = 1;
	char client_opt;
	
    // esperar a que el cliente se conecte correctamente al servidor y abrir el controlador

    
    // ----------------------------------------- inicio controlador del cliente -----------------------------------------
    while (proceed){
	
        while (waitingForServerResponse == 1) {}
	printf("1 -> Chat with everyone in the chat (Broadcasting)\n2 -> Send a private message\n3 -> Change status\n4 -> List connected users in the chat system\n5 -> Deploy info from a particular user\n6 -> Help\n7 -> Exit\nEnter the option: ");
        request->Clear();
        cin>>client_opt;
	
	// para ingresar las opciones en cliente
        switch (client_opt){
		
		// ingresar un mensaje para todos
		case '1':{
				std::string message;
				cout<<"Enter the message to be sent: ";
				cin>>message;
				chat::newMessage *m_new = new chat::newMessage();
				m_new->set_message_type(1);
				m_new->set_sender(argv[1]);
				m_new->set_message(message);
				request->set_option(4);
				request->set_allocated_message(m_new);
				request->SerializeToString(&message_serialized);
				strcpy(buffer, message_serialized.c_str());
				send(sockfd, buffer, message_serialized.size() + 1, 0);
				waitingForServerResponse = 1;
                		break;
		}
			
		// ingresar un mensaje privado
		case '2':{
				std::string recipient, message;
				printf("Enter the username of the recipient and the message in this format <username> <message>: ");
				cin>>recipient;
				cin>>message;
				chat::newMessage *m_new = new chat::newMessage();
				m_new->set_message_type(0);
				m_new->set_sender(argv[1]);
				m_new->set_recipient(recipient);
				m_new->set_message(message);
				request->set_option(4);
				request->set_allocated_message(m_new);
				request->SerializeToString(&message_serialized);
				strcpy(buffer, message_serialized.c_str());
				send(sockfd, buffer, message_serialized.size() + 1, 0);
				waitingForServerResponse = 1;
				break;
		}
			
		// ingresar un nuevo estatus
		case '3':{
				chat::ChangeStatus *newStatus = new chat::ChangeStatus();
				printf("Select between these options\n1 -> ACTIVE\n2 -> OCCUPATED\n3 -> INACTIVE\nEnter the new status: ");
				std::string op;cin>>op;
				if (op=="1"||op=="ACTIVE"||op=="active"){
					newStatus->set_newstatus(1);
				}
				else if (op=="2"||op=="OCCUPATED"||op=="occupated"){
					newStatus->set_newstatus(2);
				}
				else if (op=="3"||op=="INACTIVE"||op=="inactive"){
					newStatus->set_newstatus(3);
				}
				else{
					printf("The value entered is invalid\n");
					break;
				}
				newStatus->set_username(argv[1]);
				request->set_option(3);
				request->set_allocated_status(newStatus);
				request->SerializeToString(&message_serialized);
				strcpy(buffer, message_serialized.c_str());
				send(sockfd, buffer, message_serialized.size() + 1, 0);
				waitingForServerResponse = 1;
				break;
		}
			
		// mostrar a todos los usuarios conectados
            	case '4':{
				chat::UserInfoRequest *info = new chat::UserInfoRequest();
				info->set_type_request(1);
				request->set_option(2);
				request->set_allocated_inforequest(info);
				request->SerializeToString(&message_serialized);
				strcpy(buffer, message_serialized.c_str());
				send(sockfd, buffer, message_serialized.size() + 1, 0);
				waitingForServerResponse = 1;
                		break;
		}
		
		// solicitar informacion de usuarios
		case '5':{
				chat::UserInfoRequest *info = new chat::UserInfoRequest();
				printf("Enter the username: ");
				cin>>buffer;
				info->Clear();
				info->set_type_request(0);
				info->set_user(buffer);
				request->set_option(2);
				request->set_allocated_inforequest(info);
				request->SerializeToString(&message_serialized);
				strcpy(buffer, message_serialized.c_str());
				send(sockfd, buffer, message_serialized.size() + 1, 0);
				waitingForServerResponse = 1;
                		break;
		}
		
		// cerrar sesion
		case '7':{
				proceed = 0;
                		break;
            	}
			
            	default:{
                	break;
            	}
        }
    }
	// al cerrar sesion, se destruye el hilo
	pthread_cancel(thread_id);
	printf("Thanks for using this server chat %s\nSee ya soon!",argv[1]);
	return 0;
}
