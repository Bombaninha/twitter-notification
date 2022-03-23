#include "client.hpp"
#include "../ui/ui.hpp"

#include <ncurses.h>
#include <iostream>
#include <sys/socket.h>
#include <string.h>
#include <sys/time.h>


#include <command/command.hpp>

Client::Client(std::string profile, std::string serverHost, int serverPort) {
    this->serverAddress = serverHost;
    this->serverPort = serverPort;
    this->profile = profile;
    this->running = true;
}

Client::~Client() {
    delete this->serverConnection;
}

void Client::run() {
    std::thread uiThread([this] {
        this->handleUI();
    });

    std::thread serverThread([this] {
        this->handleServer();
    });

    uiThread.join();
    serverThread.join();   
}

void Client::connectToServer() {
    std::string response = "Connecting to server...";
    responses.push(response);

    serverConnection = new Connection(serverAddress, serverPort);

    Command connectCommand(COMMAND_CONNECT, this->profile);

    serverConnection->sendCommand(connectCommand);

    std::string serverResponse = "";


    const clock_t begin_time = clock();
    do {
        //check if time limit is reached
        if((float( clock () - begin_time ) /  CLOCKS_PER_SEC) >= 1 ){
            break;
        }

        serverResponse = serverConnection->listenToServer();
        
        if (serverResponse != "")
            responses.push(serverResponse);

    } while(serverResponse == ""); 


    if (serverResponse.substr(0, 8) == "REDIRECT") {
        Command responseCommand(serverResponse);

        delete serverConnection;
        this->serverConnection = new Connection(serverAddress, stoi(responseCommand.getData()));

        Command noOp(NO_OPERATION, "");
        serverConnection->sendCommand(noOp);

        response = "Connected to server";
        responses.push(response);
    } else {
        response = "Could not connect to server";
        responses.push(response);
        running = false;
        pthread_t thread_id = pthread_self();
        printf("Exiting socket and thread: %d\n", (int)thread_id);
        pthread_exit(NULL);
    }
}

void Client::handleServer() {
    connectToServer();


    while(running) {
        if (commands.size() > 0) {
            std::string command = commands.front();
            commands.pop();

            if (strcasecmp(command.substr(0, 6).c_str(), "follow") == 0) {
                Command followCommand(COMMAND_FOLLOW, command.substr(7));
                serverConnection->sendCommand(followCommand);
            }

            if (strcasecmp(command.substr(0, 4).c_str(), "send") == 0) {
                if (command.substr(5).length() > 128) {
                    responses.push("Message too long");                    
                } else {
                    Command sendCommand(COMMAND_SEND, command.substr(5));
                    serverConnection->sendCommand(sendCommand);
                }
            }
            if (strcasecmp(command.substr(0, 4).c_str(), "exit") == 0) {
                Command exitCommand(COMMAND_EXIT, command.substr(4));
                serverConnection->sendCommand(exitCommand);
                running = false;
            }
        }
        //listen even when there are no commands to be sent
        std::string responseFromServer = serverConnection->listenToServer();
        if (responseFromServer != "") {
            Command recieved = Command(responseFromServer);

            if (recieved.getType() == COMMAND_ERROR) {
                responses.push(responseFromServer);
            } else {
                responses.push(recieved.getData());
            }
        }
    }
}

void Client::handleUI() {
    std::string userInput = "";
    std::vector<std::string> outputContent;

    WINDOW *input, *output, *help;

    initUI(&input, &output, &help, this->profile);

    int maxContent = getmaxy(output) - 2;

    clearInput(&userInput, input);

    while (running) {
        if (responses.size() > 0) {
            std::string response = responses.front();
            responses.pop();

            if (outputContent.size() == maxContent) {
                outputContent.erase(outputContent.begin());
            }

            outputContent.push_back(response);

            updateOutput(output, outputContent);
        }

        if (kbhit()) {
            char c = getch();

            if (c == '\n') {
                if (strcasecmp(userInput.c_str(), "exit") == 0) {
                    commands.push(userInput);
                    continue;
                }

                processCommand(userInput);

                clearInput(&userInput, input);

                continue;
            }

            if (c == 27) {
                clearInput(&userInput, input);
                continue;
            }

            if (c == 127) {
                if (!userInput.empty()) {
                    userInput.pop_back();
                    mvwprintw(input, 1, 3 + userInput.length(), " ");
                    wrefresh(input);
                }

                continue;
            }

            if (isprint(c)) {
                userInput += c;
                mvwprintw(input, 1, 3 + userInput.length() - 1, "%c", c);
                wrefresh(input);
            }
        }
    }

    destroyUI(input, output, help);
}

void Client::updateOutput(WINDOW* output, std::vector<std::string>& outputContent) {
    for (int i = 0; i < outputContent.size(); i++) {
        mvwprintw(output, i + 1, 1, outputContent[i].c_str());
    }
    
    wrefresh(output);
}

void Client::processCommand(std::string command) {
    if (strcasecmp(command.substr(0, 6).c_str(), "follow") == 0) {
        commands.push(command);
        return;
    }

    if (strcasecmp(command.substr(0, 4).c_str(), "send") == 0) {
        commands.push(command);
        return;
    }

    responses.push("invalid command: " + command);
}