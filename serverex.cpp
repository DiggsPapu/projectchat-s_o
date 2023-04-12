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
#include "proyect.pb.h"
using namespace std;
// Struct of user
struct User{
    int socketFd;
    std::string username;
    char ip[INET_ADDRSTRLEN];
    int status;
};
// Client list
std::unordered_map<std::string, User*> clients;
void SendErrorResponse(int socketFd, std::string errorMsg)
{
    std::string msgSerialized;
    chat::ServerResponse *errorMessage = new chat::ServerResponse();
    errorMessage->set_option(1);
    errorMessage->set_code(400);
	errorMessage->set_servermessage("Error");
    errorMessage->SerializeToString(&msgSerialized);
    char buffer[msgSerialized.size() + 1];
    strcpy(buffer, msgSerialized.c_str());
    send(socketFd, buffer, sizeof buffer, 0);
}
void *ThreadWork(void *params)
{
    struct User user;
    struct User *newClientParams = (struct User *)params;
    int socketFd = newClientParams->socketFd;
    char buffer[8192];
    // Server Structs
    std::string msgSerialized;
    chat::UserRequest *request = new chat::UserRequest();
    chat::ServerResponse *response = new chat::ServerResponse();
    while (1){
        response->Clear();
        if (recv(socketFd, buffer, 8192, 0) < 1){
            if (recv(socketFd, buffer, 8192, 0) == 0){
                std::cout<<std::endl<<"__LOGGING OUT__\n"<< "The client named: "<< user.username<< " has logged out"<< std::endl;
				clients.erase(user.username);
            }
            break;
        }
        request->ParseFromString(buffer);
		switch (request->option()){
			case 1:{
				std::cout<<std::endl<<"__RECEIVED INFO__\nUsername: "<<request->newuser().username()<<"		ip: "<<request->newuser().ip();
				if (clients.count(request->newuser().username()) > 0)
				{
					std::cout<<std::endl<< "ERROR: Username already exists" <<std::endl;
					SendErrorResponse(socketFd, "ERROR: Username already exists");
					break;
				}
				response->set_option(1);
				response->set_servermessage("SUCCESS: register");
				response->set_code(200);
				response->SerializeToString(&msgSerialized);
				strcpy(buffer, msgSerialized.c_str());
				send(socketFd, buffer, msgSerialized.size() + 1, 0);
				std::cout<<std::endl<<"SUCCESS:The user"<<user.username<<" was added with the socket: "<<socketFd<<std::endl;
				user.username = request->newuser().username();
				user.socketFd = socketFd;
				user.status = 1;
				strcpy(user.ip, newClientParams->ip);
				clients[user.username] = &user;
				break;
			}
			case 2:{
                if(request->inforequest().type_request()){
                    chat::AllConnectedUsers *users = new chat::AllConnectedUsers();
                    for(auto i:clients){
                        chat::UserInfo *userI = new chat::UserInfo();
                        userI->set_username(i.second->username);
                        userI->set_status(i.second->status);
                        userI->set_ip(i.second->ip);
                        users->add_connectedusers()->CopyFrom(*userI);
                    }
                    response->set_servermessage("SUCCESS: userinfo of all connected clients");
                    response->set_allocated_connectedusers(users);
                    response->set_option(2);
                    response->set_code(200);
                    response->SerializeToString(&msgSerialized);
                    strcpy(buffer, msgSerialized.c_str());
                    send(socketFd, buffer, msgSerialized.size() + 1, 0);
                    std::cout<<"\n__ALL CONNECTED CLIENTS__\nUser:"<<user.username<<"requested to show info of all connected clients\n";
                }
                else{
                    chat::UserInfo *userI = new chat::UserInfo();
                    try{
                        userI->set_username(clients[request->inforequest().user()]->username);                    
                        userI->set_ip(clients[request->inforequest().user()]->ip);
                        userI->set_status(clients[request->inforequest().user()]->status);
                        response->set_allocated_userinforesponse(userI);
                        response->set_servermessage("SUCCESS: userinfo of "+request->inforequest().user());
                        response->set_code(200);
                        response->set_option(2);
                        response->SerializeToString(&msgSerialized);
                        strcpy(buffer, msgSerialized.c_str());
                        send(socketFd, buffer, msgSerialized.size() + 1, 0);
                        std::cout<<"\n__USER INFO SOLICITUDE__\nUser: "<<user.username<<" requested info of ->"<<request->inforequest().user()<<"\nSUCCESS: userinfo of "<<request->inforequest().user()<<std::endl;
                    }
                    catch(const std::exception& e){
                        response->set_servermessage("ERROR: userinfo of "+request->inforequest().user());
                        response->set_code(400);
                        response->SerializeToString(&msgSerialized);
                        strcpy(buffer, msgSerialized.c_str());
                        send(socketFd, buffer, msgSerialized.size() + 1, 0);
                        std::cout<<"\n__USER INFO SOLICITUDE__\nUser: "<<user.username<<" requested info of ->"<<request->inforequest().user()<<"\nERROR: userinfo of "<<request->inforequest().user()<<std::endl;
                    }
                    
                }
				break;
			}
			case 3:{
                clients[request->status().username()]->status = request->status().newstatus();
                std::cout<<"\n__USER CHANGE STATUS SOLICITUDE__\nUser: "<<user.username<<" requested to change status.\nSUCCESS:status has changed successfully\n";
                chat::ChangeStatus *sStatus = new chat::ChangeStatus();
                sStatus->set_username(request->status().username());
                sStatus->set_newstatus(request->status().newstatus());
                response->set_allocated_change(sStatus);
                response->set_servermessage("\nSUCCESS:\nUser: "+request->status().username()+" has changed status successfully\n");
                response->set_code(200);
                response->set_option(3);
                response->SerializeToString(&msgSerialized);
                strcpy(buffer, msgSerialized.c_str());
                send(socketFd, buffer, msgSerialized.size() + 1, 0);
				break;
			}
			case 4:{
                if(request->message().message_type()){}
                else{
                    std::cout<<"\n__SENDING PRIVATE MESSAGE__\nUser: "<<request->message().sender()<<" is trying to send a private message to ->"<<request->message().recipient();
                    if(clients.count(request->message().recipient()) > 0){
                        chat::newMessage *message = new chat::newMessage();
                        message->set_sender(user.username);
                        message->set_recipient(request->message().recipient());
                        message->set_message(request->message().message());
                        response->set_allocated_message(message);
                        response->set_servermessage("\nUser: "+request->message().sender()+" sends you a message\n");
                        response->set_code(200);
                        response->set_option(4);
                        response->SerializeToString(&msgSerialized);
                        strcpy(buffer, msgSerialized.c_str());
                        send(clients[request->message().recipient()]->socketFd, buffer, msgSerialized.size() + 1, 0);
                        response->Clear();
                        chat::newMessage *response_message = new chat::newMessage();
                        response_message->set_message("");
                        response->set_allocated_message(response_message);
                        response->set_servermessage("\nSUCCESS: You have sent the private message to "+request->message().recipient()+" successfully\n");
                        response->set_code(200);
                        response->set_option(4);
                        response->SerializeToString(&msgSerialized);
                        strcpy(buffer, msgSerialized.c_str());
                        send(socketFd, buffer, msgSerialized.size() + 1, 0);
                        std::cout<<"\nSUCCESS:User: "+request->message().sender()+" has sent the message successfully ->"+request->message().recipient()+"\n";
                    }
                    else{
                        chat::newMessage *message = new chat::newMessage();
                        message->set_sender(user.username);
                        message->set_recipient(request->message().recipient());
                        response->set_allocated_message(message);
                        response->set_servermessage("\nERROR:\nUser: "+request->message().sender()+" tried to send a message to an unexisting user ->"+request->message().recipient()+"\n");
                        response->set_code(400);
                        response->set_option(4);
                        response->SerializeToString(&msgSerialized);
                        strcpy(buffer, msgSerialized.c_str());
                        send(socketFd, buffer, msgSerialized.size() + 1, 0);
                        std::cout<<"\nERROR:\nUser: "+request->message().sender()+" tried to send a message to an unexisting user ->"+request->message().recipient()+"\n";
                    }
                }
				break;
			}
			case 5:{
				break;
			}
			default:{
				break;
			}
		}
	}
    return params;
}

int main(int argc, char const* argv[]){
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    //Cuando no se indica el puerto del server
    if (argc != 2){
        fprintf(stderr, "Use: server <server port>\n");
        return 1;
    }
    long port = strtol(argv[1], NULL, 10);
    sockaddr_in server, incoming_conn;
    socklen_t new_conn_size;
    int socket_fd, new_conn_fd;
    char incoming_conn_addr[INET_ADDRSTRLEN];
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = INADDR_ANY;
    memset(server.sin_zero, 0, sizeof server.sin_zero);
    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        fprintf(stderr, "ERROR: create socket\n");
        return 1;
    }
    if (bind(socket_fd, (struct sockaddr *)&server, sizeof(server)) == -1){
        close(socket_fd);
        fprintf(stderr, "ERROR: bind IP to socket.\n");
        return 2;
    }
    if (listen(socket_fd, 5) == -1){
        close(socket_fd);
        fprintf(stderr, "ERROR: listen socket\n");
        return 3;
    }
    printf("SUCCESS: listening on port-> %ld\n", port);
    while (1){
        new_conn_size = sizeof incoming_conn;
        new_conn_fd = accept(socket_fd, (struct sockaddr *)&incoming_conn, &new_conn_size);
        if (new_conn_fd == -1){
            perror("ERROR: accept socket incomming connection\n");
            continue;
        }
        struct User newClient;
        newClient.socketFd = new_conn_fd;
        inet_ntop(AF_INET, &(incoming_conn.sin_addr), newClient.ip, INET_ADDRSTRLEN);
        pthread_t thread_id;
        pthread_attr_t attrs;
        pthread_attr_init(&attrs);
        pthread_create(&thread_id, &attrs, ThreadWork, (void *)&newClient);
    }
    google::protobuf::ShutdownProtobufLibrary();
	return 0;
}