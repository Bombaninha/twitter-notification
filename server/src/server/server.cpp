#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <utility>
#include "server.hpp"

#define MAX_CONNECTIONS_OF_SAME_USER 2

pthread_mutex_t readAndWriteMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t readMutex = PTHREAD_MUTEX_INITIALIZER;

int readers = 0;

void sharedReaderLock() {
	pthread_mutex_lock(&readMutex);
	readers++;
	if(readers == 1)
		pthread_mutex_lock(&readAndWriteMutex);
	pthread_mutex_unlock(&readMutex);
}

void sharedReaderUnlock(){
	pthread_mutex_lock(&readMutex);
	readers--;
	if(readers == 0)
		pthread_mutex_unlock(&readAndWriteMutex);
	pthread_mutex_unlock(&readMutex);
}

Server::Server(int port) {
    this->sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (this->sockfd < 0) {
        throw std::runtime_error("Could not create socket");
    }

    struct sockaddr_in server_addr;

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(server_addr.sin_zero), 8);

    if (bind(this->sockfd, (struct sockaddr *) &server_addr, sizeof(struct sockaddr)) < 0) {
        throw std::runtime_error("Could not bind socket");
    }
}

Server::~Server() {
    for (auto row : this->masterTable) {
        delete row.second;
    }

    for (auto thread : this->clientThreads) {
        delete thread;
    }

    for (auto client : this->clients) {
        delete client;
    }

    close(this->sockfd);
}

void Server::serverLoop() {
    char buffer[256];
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(struct sockaddr_in);

    while(true) {
        int recieve = recvfrom(this->sockfd, buffer, 256, 0, (struct sockaddr *) &client_addr, &client_addr_len);

        if (recieve < 0) {
            std::cout << "Error recieving data" << std::endl;
            continue;
        }

        std::cout << "Recieved: " << buffer << std::endl;

        Command command = Command(std::string(buffer));

        Command response = this->execute(command);

        int send = sendto(this->sockfd, std::string(response).c_str(), std::string(response).length(), 0, (struct sockaddr *) &client_addr, client_addr_len);

        if (send < 0) {
            std::cout << "Error sending data" << std::endl;
            continue;
        }

        std::cout << "Sent: " << std::string(response) << std::endl;

        bzero(buffer, 256);
    }
}

std::thread Server::run() {
    std::thread server_thread(&Server::serverLoop, this);

    return server_thread;
}

Command Server::execute(Command command) {
    Command response;

    switch(command.getType()) {
        case COMMAND_CONNECT:
            this->createClientSocket(this->nextClientPort, command.getData());
            response = Command(COMMAND_REDIRECT, std::to_string(this->nextClientPort));
            this->nextClientPort++;
            break;
        case COMMAND_FOLLOW:
            response = Command(COMMAND_FOLLOW, "Following");
            break;
        case COMMAND_SEND:
            response = Command(COMMAND_SEND, "Sending");
            break;
    }

    return response;
}

void Server::createClientSocket(int port, std::string profile) {
    ClientSocket *clientSocket = new ClientSocket(port, profile);
    TableRow *currentTableRow;

    sharedReaderLock();
    bool usernameDoesNotExist = this->masterTable.find(profile) == this->masterTable.end();
    sharedReaderUnlock();

    if(usernameDoesNotExist) {
        pthread_mutex_lock(&readAndWriteMutex);
        TableRow* newTableRow;
		this->masterTable.insert(std::make_pair(profile, newTableRow));	
        pthread_mutex_unlock(&readAndWriteMutex);
    } 
    
    std::cout << "User " + profile + " is connecting...";

	sharedReaderLock();
	currentTableRow = this->masterTable.find(profile)->second;
	sharedReaderUnlock();

    int currentRowActiveSessions = currentTableRow->getActiveSessions();

    if (currentRowActiveSessions >= MAX_CONNECTIONS_OF_SAME_USER) {
        std::cout << "\n denied: there are already 2 active sessions!\n" << std::endl;
		// fechar conexao
	} else {
		currentTableRow->startSession();
		std::cout << " connected." << std::endl;
	}

    std::thread* client_thread = new std::thread([clientSocket]() {
        clientSocket->run();
    });
    
    this->clients.push_back(clientSocket);
    this->clientThreads.push_back(client_thread);
}