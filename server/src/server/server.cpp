#include <iostream>
#include <fstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <utility>
#include <list>
#include <map>
#include <ctime>

#include "server.hpp"

#include "command/command.hpp"

#include "../table/tableRow.hpp"

#define MAX_CONNECTIONS_OF_SAME_USER 2

pthread_mutex_t readAndWriteMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t readMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

std::map<std::string, TableRow*> masterTable; 

int readers = 0;

bool electionStarted = false;

void saveBackup() {
	std::list<std::string> followers;
	std::string username;
	TableRow* tableRow;
    std::string fileName = "backup_table.txt";
	std::ofstream tableFile;

	tableFile.open("backup_table.txt", std::ios::out | std::ios::trunc); 

    if(tableFile.is_open()) {
        for(auto const& row : masterTable) {
            username = row.first;
            tableRow = row.second;

		    tableFile << username;
		    tableFile << "/";

            followers = tableRow->getFollowers();
            for (std::string follower : followers) {
                tableFile << follower;
                tableFile << ",";
            }

            tableFile << "\n";
        } 
        tableFile.close(); 
    } else {
        std::cout << "ERROR opening the file" << std::endl;
    }
} 

inline bool fileExists(const std::string& name) {
    return (access(name.c_str(), F_OK) != -1);
}

std::map<std::string, TableRow*> loadBackup() {
	char* line_ptr_aux;
	char* token;
	TableRow* tableRow;
    std::map<std::string, TableRow*> masterTable;

	if(fileExists("backup_table.txt")) {
		std::cout << "Restaurando dados..." << std::endl;
		fflush(stdout);
		std::ifstream tableFile("backup_table.txt");

		for(std::string line; getline(tableFile, line);) {
			char* line_ptr = strdup(line.c_str());

			tableRow = new TableRow();
			strcpy(line_ptr, line.c_str());

			token = strtok_r(line_ptr, "/", &line_ptr_aux);
			std::string username(token);

			token = strtok_r(NULL, ",", &line_ptr_aux);
            std::cout << username << std::endl;
			while(token != NULL) {
				std::string follower(token);
                if(username != follower) {
				    tableRow->addFollower(follower);
                }
				token = strtok_r(NULL, ",", &line_ptr_aux);
			}

			pthread_mutex_lock(&readAndWriteMutex);
			masterTable.insert(std::make_pair(username, tableRow));
			pthread_mutex_unlock(&readAndWriteMutex);
		}
		tableFile.close(); 
	} else {
		printf("Backup table not found. Creating new. \n");
		fflush(stdout);
	}

	return masterTable;
}

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

    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 100000;
    if (setsockopt(this->sockfd, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) {
        perror("Error");
    }

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

    masterTable = loadBackup();
}

Server::Server(int port, std::string primaryHost, int primaryPort) {
    this->sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 100000;
    if (setsockopt(this->sockfd, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) {
        perror("Error");
    }


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

    masterTable = loadBackup();

    this->primaryPort = primaryPort;
    this->primaryConnection = new Connection(primaryHost, primaryPort);

    int pid = getpid();

    char hostname[50];

    int n = gethostname(hostname, 50);

    if (n < 0) {
        throw std::runtime_error("Could not get hostname");
    }

    Command aliveComand(COMMAND_ALIVE, std::string(hostname) + ":" + std::to_string(port) + "," + std::to_string(pid));
    this->primaryConnection->sendCommand(aliveComand);

    auto response = this->primaryConnection->listenToServer();

    while (response != "") {
        std::cout << "Recieved: " << response << std::endl;

        Command command(response);
        
        if (command.getType() == COMMAND_BACKUP) {
            std::string data = command.getData();

            std::string hostname = data.substr(0, data.find(":"));
            auto temp = data.substr(data.find(":") + 1);
            std::string port = temp.substr(0, temp.find(","));
            std::string pid = temp.substr(temp.find(",") + 1);

            this->backupServers.push_back(std::make_tuple(atoi(pid.c_str()), hostname, atoi(port.c_str())));
        }

        response = this->primaryConnection->listenToServer();
    }
}

Server::~Server() {
    for (auto row : masterTable) {
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

void Server::setPrimary() {
    this->isPrimary = true;
}

void Server::serverLoop() {
    if (!this->isPrimary) {
        this->backupLoop();

        for (auto user : masterTable) {
            for (auto session : user.second->sessions()) {
                masterTable[user.first]->closeSession(session.first, session.second);

                auto host = session.first;
                auto port = session.second;
                struct sockaddr_in client_addr;

                std::cout << "Connecting to " << user.first << " at " << host << ":" << port << std::endl;

                this->createClientSocket(host, port, user.first, client_addr);
            }
        }
    }

    this->primaryLoop();
}

void Server::changePort() {
    close(this->sockfd);

    this->sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 100000;
    if (setsockopt(this->sockfd, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) {
        perror("Error");
    }

    if (this->sockfd < 0) {
        throw std::runtime_error("Could not create socket");
    }

    struct sockaddr_in server_addr;

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(primaryPort);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(server_addr.sin_zero), 8);

    if (bind(this->sockfd, (struct sockaddr *) &server_addr, sizeof(struct sockaddr)) < 0) {
        throw std::runtime_error("Could not bind socket");
    }
}

void Server::backupLoop() {
    char buffer[256];
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(struct sockaddr_in);

    std::time_t lastTime = std::time(nullptr);

    while (!this->isPrimary) {
        if (std::time(nullptr) - lastTime > 3 && !electionStarted) {
            Command ticCommand(COMMAND_TIC, "");

            this->primaryConnection->sendCommand(ticCommand);

            std::string responseString = this->primaryConnection->listenToServer();

            if (responseString.empty()) {
                if (!electionStarted) {
                    electionStarted = true;
                    std::cout << "Starting election" << std::endl;

                    std::thread electionThread(&Server::electionLoop, this);
                    electionThread.detach();
                }
            }

            lastTime = std::time(nullptr);
        }

        bzero(buffer, 256);

        int recieve = recvfrom(this->sockfd, buffer, 256, 0, (struct sockaddr *) &client_addr, &client_addr_len);

        if (recieve > 0) {
            if (std::string(buffer) != "TOC") {
                std::cout << "Recieved: " << buffer << std::endl;
            }

            Command command = Command(std::string(buffer));

            if (command.getType() == NO_OPERATION) {
                continue;
            }

            Command response = this->execute(command, client_addr);

            int send = sendto(this->sockfd, std::string(response).c_str(), std::string(response).length(), 0, (struct sockaddr *) &client_addr, client_addr_len);

            if (send < 0) {
                std::cout << "Error sending data" << std::endl;
                continue;
            }

            if (std::string(buffer) != "TIC") {
                std::cout << "Sent: " << std::string(response) << std::endl;
            }
        }
    }
}

void Server::primaryLoop() {
    char buffer[256];
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(struct sockaddr_in);

    while(true) {
        bzero(buffer, 256);

        int recieve = recvfrom(this->sockfd, buffer, 256, 0, (struct sockaddr *) &client_addr, &client_addr_len);

        if (recieve < 0) {
            // std::cout << "Error recieving data" << std::endl;
            continue;
        }

        // if (std::string(buffer) != "TIC") {
            std::cout << "Recieved: " << buffer << std::endl;
        // }

        Command command = Command(std::string(buffer));

        if (command.getType() == NO_OPERATION) {
            continue;
        }

        Command response = this->execute(command, client_addr);

        int send = sendto(this->sockfd, std::string(response).c_str(), std::string(response).length(), 0, (struct sockaddr *) &client_addr, client_addr_len);

        if (send < 0) {
            std::cout << "Error sending data" << std::endl;
            continue;
        }

        // if (std::string(buffer) != "TIC") {
            std::cout << "Sent: " << std::string(response) << std::endl;
        // }
    }
}

void Server::electionLoop() {
    int pidSelf = getpid();
    bool recievedAnswer = false;

    for (auto backup : this->backupServers) {
        int pidBackup = std::get<0>(backup);

        if (pidSelf > pidBackup) {
            continue;
        }

        std::string hostname = std::get<1>(backup);
        int port = std::get<2>(backup);

        Connection connection(hostname, port);

        Command electionCommand(COMMAND_ELECTION, std::to_string(pidSelf));

        connection.sendCommand(electionCommand);

        std::string response = connection.listenToServer();

        if (response != "") {
            recievedAnswer = true;
            break;
        }
    }

    if (!recievedAnswer) {
        std::cout << "I am the coordinator" << std::endl;

        this->changePort();

        for (auto backup : this->backupServers) {
            std::string hostname = std::get<1>(backup);
            int port = std::get<2>(backup);

            std::cout << "Sending coordinator announcement to " << hostname << ":" << port << std::endl;

            Connection connection(hostname, port);

            Command coordinatorCommand(COMMAND_COORDINATOR, std::to_string(pidSelf));

            connection.sendCommand(coordinatorCommand);

            std::string response = connection.listenToServer();

            while (response == "") {
                std::cout << "Sending coordinator announcement to " << hostname << ":" << port << std::endl;

                connection.sendCommand(coordinatorCommand);
                response = connection.listenToServer();
            }
        }

        this->isPrimary = true;
    }

    std::cout << "Election ended" << std::endl;
}

std::thread Server::run() {
    std::thread server_thread(&Server::serverLoop, this);

    return server_thread;
}

Command Server::execute(Command command, struct sockaddr_in client_addr) {
    Command response;

    switch(command.getType()) {
        case COMMAND_CONNECT:
            {

                char *ip = inet_ntoa(client_addr.sin_addr);
            // Se já existe 2 active sessions
                sharedReaderLock();
                bool usernameDoesNotExist = masterTable.find(command.getData()) == masterTable.end();
                sharedReaderUnlock();

                if(usernameDoesNotExist) {
                    pthread_mutex_lock(&readAndWriteMutex);
                    TableRow* newTableRow = new TableRow();
                    masterTable.insert(std::make_pair(command.getData(), newTableRow));	
                    pthread_mutex_unlock(&readAndWriteMutex);
                    saveBackup();

                    this->createClientSocket(ip, this->nextClientPort, command.getData(), client_addr);
                    response = Command(COMMAND_REDIRECT, std::to_string(this->nextClientPort));
                    pthread_mutex_lock(&mutex);
                    this->nextClientPort++;
                    pthread_mutex_unlock(&mutex);
                } else {
                    sharedReaderLock();
                    TableRow *currentTableRow;
                    currentTableRow = masterTable.find(command.getData())->second;

                    int currentRowActiveSessions = currentTableRow->getActiveSessions();

                    sharedReaderUnlock();

                    if (currentRowActiveSessions >= MAX_CONNECTIONS_OF_SAME_USER){
                        response = Command(COMMAND_ERROR, "\n denied: there are already 2 active sessions!\n" );
                    } else {
                        this->createClientSocket(ip, this->nextClientPort, command.getData(), client_addr);
                        response = Command(COMMAND_REDIRECT, std::to_string(this->nextClientPort));
                        pthread_mutex_lock(&mutex);
                        this->nextClientPort++;
                        pthread_mutex_unlock(&mutex);
                    }
                }
            break;
            }
        case COMMAND_FOLLOW:
        {
            response = Command(COMMAND_FOLLOW, "Following");
            break;
        }
        case COMMAND_SEND:
        {
            response = Command(COMMAND_SEND, "Sending");
            break;    
        }
        case COMMAND_ALIVE:
        {
            std::string data = command.getData();

            std::string hostname = data.substr(0, data.find(":"));
            auto temp = data.substr(data.find(":") + 1);
            std::string port = temp.substr(0, temp.find(","));
            std::string pid = temp.substr(temp.find(",") + 1);

            Command newBackup(COMMAND_BACKUP, data);

            for (auto backup : this->backupServers) {
                Connection connection(std::get<1>(backup), std::get<2>(backup));

                connection.sendCommand(newBackup);
            }

            for (auto backup : this->backupServers) {
                auto pid = std::get<0>(backup);
                auto hostname = std::get<1>(backup);
                auto port = std::get<2>(backup);

                Command existingBackup(COMMAND_BACKUP, hostname + ":" + std::to_string(port) + "," + std::to_string(pid));

                struct sockaddr_in backupAddress;

                int n = sendto(
                    this->sockfd,
                    std::string(existingBackup).c_str(),
                    std::string(existingBackup).length(),
                    0,
                    (struct sockaddr*) &client_addr,
                    sizeof(client_addr));

                std::cout << "Sent: " << std::string(existingBackup) << std::endl;

                if (n < 0) {
                    //return Command(COMMAND_ERROR, "Could not send command");
                    std::cout << "Could not send command to backup" << std::endl;
                }
            }

            this->backupServers.push_back(std::make_tuple(atoi(pid.c_str()), hostname, atoi(port.c_str())));

            response = Command(COMMAND_ALIVE, "Alive");
            break;
        }
        case COMMAND_BACKUP: {
            std::string data = command.getData();

            std::string hostname = data.substr(0, data.find(":"));
            auto temp = data.substr(data.find(":") + 1);
            std::string port = temp.substr(0, temp.find(","));
            std::string pid = temp.substr(temp.find(",") + 1);

            this->backupServers.push_back(std::make_tuple(atoi(pid.c_str()), hostname, atoi(port.c_str())));

            response = Command(NO_OPERATION, "");
            break;
        }
        case COMMAND_TIC: {
            response = Command(COMMAND_TOC, "");
            break;
        }
        case COMMAND_ELECTION: {
            int pidSelf = getpid();

            if (pidSelf > atoi(command.getData().c_str())) {
                if (!electionStarted) {
                    electionStarted = true;
                    std::thread electionThread(&Server::electionLoop, this);
                    electionThread.detach();
                }

                response = Command(COMMAND_ANSWER, "");
                break;
            }

            break;
        }
        case COMMAND_ANSWER: {
            response = Command(COMMAND_ANSWER, "");
            break;
        }
        case COMMAND_COORDINATOR: {
            int coordPid = atoi(command.getData().c_str());

            std::vector<std::tuple<int, std::string, int>> newBackupList;

            for (auto backup : this->backupServers) {
                if (std::get<0>(backup) != coordPid) {
                    newBackupList.push_back(backup);
                }
                // else {
                //     this->primaryConnection = new Connection(std::get<1>(backup), std::get<2>(backup));
                // }
            }

            this->backupServers = newBackupList;

            electionStarted = false;

            response = Command(NO_OPERATION, "");
            
            break;
        }
        case COMMAND_REPLICATE_CONNECT: {
            std::string data = command.getData();

            std::string hostname = data.substr(0, data.find(":"));
            auto temp = data.substr(data.find(":") + 1);
            std::string port = temp.substr(0, temp.find(","));
            std::string profile = temp.substr(temp.find(",") + 1);

            sharedReaderLock();
            bool usernameDoesNotExist = masterTable.find(profile) == masterTable.end();
            sharedReaderUnlock();

            if(usernameDoesNotExist) {
                pthread_mutex_lock(&readAndWriteMutex);
                TableRow* newTableRow = new TableRow();
                masterTable.insert(std::make_pair(profile, newTableRow));	
                pthread_mutex_unlock(&readAndWriteMutex);
                saveBackup();

                sharedReaderLock();
                TableRow *currentTableRow;
                currentTableRow = masterTable.find(profile)->second;
                sharedReaderUnlock();

                // aumenta em um a quantidade de conexões do mesmo usuário
                currentTableRow->startSession(hostname, atoi(port.c_str()));

                response = Command(COMMAND_REDIRECT, std::to_string(this->nextClientPort));
                pthread_mutex_lock(&mutex);
                this->nextClientPort++;
                pthread_mutex_unlock(&mutex);
            } else {
                sharedReaderLock();
                TableRow *currentTableRow;
                currentTableRow = masterTable.find(profile)->second;

                int currentRowActiveSessions = currentTableRow->getActiveSessions();

                sharedReaderUnlock();

                if (currentRowActiveSessions >= MAX_CONNECTIONS_OF_SAME_USER){
                    response = Command(COMMAND_ERROR, "\n denied: there are already 2 active sessions!\n" );
                } else {
                    sharedReaderLock();
                    TableRow *currentTableRow;
                    currentTableRow = masterTable.find(profile)->second;
                    sharedReaderUnlock();

                    // aumenta em um a quantidade de conexões do mesmo usuário
                    currentTableRow->startSession(hostname, atoi(port.c_str()));
                    
                    response = Command(COMMAND_REDIRECT, std::to_string(this->nextClientPort));
                    pthread_mutex_lock(&mutex);
                    this->nextClientPort++;
                    pthread_mutex_unlock(&mutex);
                }
            }
            
            break;
        }
        case COMMAND_REPLICATE_DISCONNECT: {
            std::string data = command.getData();

            std::string username = data.substr(0, data.find(","));
            data = data.substr(data.find(",") + 1);
            std::string host = data.substr(0, data.find(":"));
            std::string port = data.substr(data.find(":") + 1);

            sharedReaderLock();
            TableRow *currentTableRow;
            currentTableRow = masterTable.find(username)->second;
            sharedReaderUnlock();

            currentTableRow->closeSession(host, atoi(port.c_str()));

            response = Command(NO_OPERATION, "");
            break;
        }
        case COMMAND_REPLICATE_FOLLOW: {
            std::string data = command.getData();
            std::string profile = data.substr(0, data.find(","));
            std::string follow = data.substr(data.find(",") + 1);

            sharedReaderLock();
            TableRow* currentRow = masterTable.find(profile)->second;
			// check if current user exists 
            bool currentUserExists = masterTable.find(profile) != masterTable.end();
			// check if newFollowing exists
			bool newFollowingExists = masterTable.find(follow) != masterTable.end();
			// check if currentUser is not trying to follow himself
			bool followingHimself = profile == follow;
			sharedReaderUnlock();

            //currentRow = table.find(this->profile)->second;
           // std::cout << "TEST currentRow " + table.find(this->profile)->first << std::endl;

            
            if(newFollowingExists) {
                if(!followingHimself) {
                    sharedReaderLock();
                    TableRow* followingRow = masterTable.find(follow)->second;
                    bool notDuplicateFollowing = !(followingRow->hasFollower(profile));
                    sharedReaderUnlock(); 
                    
                    if(notDuplicateFollowing) {
				        followingRow->addFollower(profile);
					    std::cout << profile + " is now following " + follow + "." << std::endl;
				    }  else {
					    std::cout << profile + " is already following " + follow + "." << std::endl;
				    }  
                    saveBackup();
                } else {
                    std::cout << profile + " is trying to follow himself." << std::endl;
                }
            } else {
               std::cout << profile + " is trying to follow an inexistent profile" << std::endl;
            }

            response = Command(NO_OPERATION, "");
        }
        case COMMAND_REPLICATE_SEND: {
            std::string data = command.getData();
            std::string profile = data.substr(0, data.find("&@"));
            std::string message = data.substr(data.find("&@") + 2);

            sharedReaderLock();
            TableRow* currentRow = masterTable.find(profile)->second;
            sharedReaderUnlock(); 

			std::list<std::string> followers = currentRow->getFollowers();

            if(followers.empty()){
                response = Command(COMMAND_ERROR, "You have no followers!");
            } else {
                //coloca na lista de msg_to_receive de todos segudores
                for (std::string follower : followers){
                    TableRow* followerRow = masterTable.find(follower)->second;
                    if (followerRow->getActiveSessions() == 0) {
                        followerRow->addNotification(profile, message);
                    }
                } 
                response = Command(COMMAND_SEND, "Your message has been sent to your followers.");    
            }
        }
    }   

    return response;
}

void Server::createClientSocket(std::string host, int port, std::string profile, struct sockaddr_in client_addr) {
    std::cout << "User " + profile + " is connecting...";

    ClientSocket *clientSocket = new ClientSocket(host, port, profile, client_addr);

    Command replicateCommand(COMMAND_REPLICATE_CONNECT, std::string(host) + ":" + std::to_string(port) + "," + profile);
    
    for (auto backup : this->backupServers) {
        Connection connection(std::get<1>(backup), std::get<2>(backup));

        connection.sendCommand(replicateCommand);
    }

	sharedReaderLock();
    TableRow *currentTableRow;
	currentTableRow = masterTable.find(profile)->second;
	sharedReaderUnlock();

    // aumenta em um a quantidade de conexões do mesmo usuário
	currentTableRow->startSession(host, port);

	std::cout << " connected." << std::endl;

    std::thread* client_thread = new std::thread([clientSocket]() {
        clientSocket->run();
    });
    
    this->clients.push_back(clientSocket);
    this->clientThreads.push_back(client_thread);
}

void Server::replicateDisconnect(std::string username, std::string host, int port) {
    Command replicateCommand(COMMAND_REPLICATE_DISCONNECT, username + "," + std::string(host) + ":" + std::to_string(port));

    for (auto backup : this->backupServers) {
        Connection connection(std::get<1>(backup), std::get<2>(backup));

        connection.sendCommand(replicateCommand);
    }
}

void Server::replicateFollow(std::string username, std::string follow) {
    Command replicateCommand(COMMAND_REPLICATE_FOLLOW, username + "," + follow);

    for (auto backup : this->backupServers) {
        Connection connection(std::get<1>(backup), std::get<2>(backup));

        connection.sendCommand(replicateCommand);
    }
}

void Server::replicateSend(std::string username, std::string message) {
    Command replicateCommand(COMMAND_REPLICATE_SEND, username + "&@" + message);

    for (auto backup : this->backupServers) {
        Connection connection(std::get<1>(backup), std::get<2>(backup));

        connection.sendCommand(replicateCommand);
    }
}