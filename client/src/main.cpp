#include <iostream>
#include <string>
#include <ncurses.h>
#include <vector>
#include <string.h>

#include "client/client.hpp"

using namespace std;

int main(int argc, char** argv) {
    if (argc != 4) {
        cout << "Usage: ./client <profile> <server address> <port>" << endl;
        return 1;
    }

    string profile = argv[1];
    string serverAddress = argv[2];
    int serverPort = stoi(argv[3]);

    Client client(profile, serverAddress, serverPort);

    client.run();

    return 0;
}