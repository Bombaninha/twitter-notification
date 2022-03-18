#include "clientSocket.hpp"

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>

ClientSocket::ClientSocket(int port) {
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

ClientSocket::~ClientSocket() {
    close(this->sockfd);
}

void ClientSocket::run() {
    char buffer[256];
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(struct sockaddr_in);

    while(true) {
        int recieve = recvfrom(this->sockfd, buffer, 256, 0, (struct sockaddr *) &client_addr, &client_addr_len);

        if (recieve < 0) {
            std::cout << "Error recieving data" << std::endl;
            continue;
        }

        std::cout << "(Client) Recieved: " << buffer << std::endl;

        Command command = Command(std::string(buffer));

        Command response = this->execute(command);

        int send = sendto(this->sockfd, std::string(response).c_str(), std::string(response).length(), 0, (struct sockaddr *) &client_addr, client_addr_len);

        if (send < 0) {
            std::cout << "Error sending data" << std::endl;
            continue;
        }

        std::cout << "(Client) Sent: " << std::string(response) << std::endl;

        bzero(buffer, 256);
    }
}

Command ClientSocket::execute(Command command) {
    if (command.getType() == COMMAND_FOLLOW) {
        return Command(COMMAND_FOLLOW, "Follow");
    }

    if (command.getType() == COMMAND_SEND) {
        return Command(COMMAND_SEND, "Send");
    }

    return Command(COMMAND_ERROR, "Command not found");
}