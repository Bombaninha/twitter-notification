#include "clientSocket.hpp"

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <map>

#include "../table/tableRow.hpp"

extern pthread_mutex_t readMutex;
extern pthread_mutex_t readAndWriteMutex;
extern int readers;
extern void sharedReaderLock();
extern void sharedReaderUnlock();

ClientSocket::ClientSocket(int port, std::string profile) {
    this->sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (this->sockfd < 0) {
        throw std::runtime_error("Could not create socket");
    }

    struct sockaddr_in server_addr;

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(server_addr.sin_zero), 8);

    // TIMEOUT
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 100000;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) {
        perror("Error");
    }

    if (bind(this->sockfd, (struct sockaddr *) &server_addr, sizeof(struct sockaddr)) < 0) {
        throw std::runtime_error("Could not bind socket");
    }

    TableRow* newTableRow = new TableRow();
	masterTable.insert(std::make_pair(profile, newTableRow));
    this->profile = profile;
}

ClientSocket::~ClientSocket() {
    close(this->sockfd);
}

void ClientSocket::run() {
    char buffer[256];
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(struct sockaddr_in);

    TableRow* currentRow = masterTable.find(this->profile)->second; 

    Command response;

    while(true) {

        /*
        for (auto row : masterTable) {
            std::cout << row.first << std::endl;
        }
        */

        //check if theres a new tweet to read
        if(currentRow->hasNewNotification()){
            std::cout << "TEVE NOTIFICACAO!" << std::endl;
            listenToNotifications(client_addr);
        } 

        int recieve = recvfrom(this->sockfd, buffer, 256, 0, (struct sockaddr *) &client_addr, &client_addr_len);

        if (recieve < 0) {
            //caiu no time out
            //std::cout << "Error recieving data" << std::endl;
            continue;
        } 
            
        std::cout << "(Client " << profile << ") Recieved: " << buffer << std::endl;

        Command command = Command(std::string(buffer));

        response = this->execute(command);

        int send = sendto(this->sockfd, std::string(response).c_str(), std::string(response).length(), 0, (struct sockaddr *) &client_addr, client_addr_len);

        if (send < 0) {
            std::cout << "Error sending data" << std::endl;
            continue;
        }

        std::cout << "(Client " << profile << ") Sent: " << std::string(response) << std::endl;

        bzero(buffer, 256);

    }
}

            //*(this->masterTable).find(this->profile)
            
            //.second.addFollower(command.getData());
/*
    for (auto row : *(this->masterTable)) {
        std::cout << row.first << std::endl;
    }
*/

Command ClientSocket::execute(Command command) {
    Command response;

    TableRow* currentRow;

    switch(command.getType()) {
        case COMMAND_FOLLOW: {
            sharedReaderLock();
			// check if current user exists 
            std::string newFollowingUsername = command.getData();
            bool currentUserExists = masterTable.find(this->profile) != masterTable.end();
			// check if newFollowing exists
			bool newFollowingExists = masterTable.find(newFollowingUsername) != masterTable.end();
			// check if currentUser is not trying to follow himself
			bool followingHimself = this->profile == newFollowingUsername;
			sharedReaderUnlock();

            //currentRow = table.find(this->profile)->second;
           // std::cout << "TEST currentRow " + table.find(this->profile)->first << std::endl;

            
            if(newFollowingExists) {
                if(!followingHimself) {
                    sharedReaderLock();
                    TableRow* followingRow = masterTable.find(newFollowingUsername)->second;
                    currentRow = masterTable.find(this->profile)->second;
                    bool notDuplicateFollowing = !(followingRow->hasFollower(this->profile));
                    sharedReaderUnlock(); 
                    
                    if(notDuplicateFollowing) {
				        followingRow->addFollower(this->profile);
					    std::cout << this->profile + " is now following " + newFollowingUsername + "." << std::endl;
                        response = Command(COMMAND_FOLLOW, "You're now following " + newFollowingUsername + ".");
				    }  else {
					    std::cout << this->profile + " is already following " + newFollowingUsername + "." << std::endl;
                        response = Command(COMMAND_ERROR, "You're already following " + newFollowingUsername + ".");
				    }  
                } else {
                    std::cout << this->profile + " is trying to follow himself." << std::endl;
                    response = Command(COMMAND_ERROR, "You're trying to follow yourself.");
                }
            } else {
               std::cout << this->profile + " is trying to follow an inexistent profile" << std::endl; 
               response = Command(COMMAND_ERROR, "You're trying to follow an inexistent profile");
            }

            /*  
            if(currentUserExists && newFollowingExists && !followingHimself) {
                sharedReaderLock();
                currentRow = masterTable.find(this->profile)->second;
                TableRow* followingRow = masterTable.find(newFollowingUsername)->second;
			
                // check if currentUser does not follow newFollowing yet
				bool notDuplicateFollowing = !(followingRow->hasFollower(this->profile));
                sharedReaderUnlock(); 
                
				if(notDuplicateFollowing) {
				    followingRow->addFollower(this->profile);
					std::cout << this->profile + " is now following " + newFollowingUsername + "." << std::endl;
				} else {
					std::cout << this->profile + " is already following " + newFollowingUsername + "." << std::endl;
				}    
            } else {
			    std::cout << this->profile + " is trying to follow " + newFollowingUsername + " but either user does not exist or " + this->profile + " is trying to follow himself." << std::endl;
			}
            */
            //response = Command(COMMAND_FOLLOW, "Following");
            break;
        }
        case COMMAND_SEND:
        {
            currentRow = masterTable.find(this->profile)->second;
			std::list<std::string> followers = currentRow->getFollowers();

            if(followers.empty()){
                response = Command(COMMAND_ERROR, "You have no followers!");
            } else {
                std::cout << "tamo aqui no teste" << std::endl;
                std::string message = command.getData();
                //coloca na lista de msg_to_receive de todos segudores
                for (std::string follower : followers){
                    std::cout << "achou um follower pelo menos!" << std::endl;
                    TableRow* followerRow = masterTable.find(follower)->second;
                    followerRow->addNotification(this->profile, message); 
                } 
                response = Command(COMMAND_SEND, "Your message has been sent to your followers.");    
            }
            break;
        }
            
        default:
        {
            response = Command(COMMAND_ERROR, "Command not found");
            break;
        }
    }

    return response;
}

void ClientSocket::listenToNotifications(struct sockaddr_in client_addr){

    std::string notification;
    Command response;
	TableRow* currentRow = masterTable.find(this->profile)->second;
    if (!(currentRow->messagesToReceive.empty())){
		//se 1 sessao ativa, pode ler e remover da lista de msg_to_receive
		if(currentRow->activeSessions == 1){
            notification = currentRow->popNotification();
            std::cout << "NOTIFICATION: " << notification << std::endl;
            response = Command(COMMAND_SEND, notification);
        } else if(currentRow->activeSessions == 2){
            response = Command(COMMAND_SEND, "Só testando aqui");
            bool wasNotificationDelivered = currentRow->getNotificationDelivered();

            if(wasNotificationDelivered){
                notification = currentRow -> popNotification();
                response = Command(COMMAND_SEND, notification);
            } else {
                currentRow->setNotificationDelivered(true);
                notification = currentRow -> getNotification();
                response = Command(COMMAND_SEND, notification);
            }       
        }
			
	} else{
        response = Command(COMMAND_ERROR, "XXXXXXXXX");
    }
    std::cout << "***********************" <<std::endl;
    std::cout << response.getType() << std::endl;
    std::cout << response.getData() << std::endl;

    std::cout << "***********************" <<std::endl;
    int send = sendto(this->sockfd, std::string(response).c_str(), std::string(response).length(), 0, (struct sockaddr *) &client_addr, sizeof(struct sockaddr_in));
    std::cout << "Send: " << send <<std::endl;


}