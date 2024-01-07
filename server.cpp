#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
// gcc program.c -lpthread -Wall -o outfile Remember to link pthread bib

const int one = 1;
int serverSocket, newSocket;
char client_message[256];
char buffer[256];

// Clients - [<name>, <socket>, <...permissions>]
std::vector<std::tuple<std::string, int, std::vector<std::string>>> clients;

void sigintHandler(int signum) {
    std::cout << "\nCTRL-C detected. Closing socket and exiting...\n";
    if (close(serverSocket) != 0) std::cout << "Couldn't close the server socket\n";
    exit(signum);
}

void setupSignalHandler() {
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = sigintHandler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;

    sigaction(SIGINT, &sigIntHandler, NULL);
}

std::tuple<std::string, int, std::vector<std::string>> getUserBySocket(int socket) {
    std::tuple<std::string, int, std::vector<std::string>> user;
    bool found = false;
    for (const auto &client : clients) {
        if (std::get<1>(client) == socket) {
            user = client;
            found = true;
            break;
        }
    }
    if (!found) {
        std::cout << "Could not authenticate the user.\n";
        return std::make_tuple("", 0, std::vector<std::string>());
    }
    return user;
}

std::tuple<std::string, int, std::vector<std::string>> getUserByName(std::string name) {
    std::tuple<std::string, int, std::vector<std::string>> user;
    bool found = false;
    for (const auto &client : clients) {
        if (std::get<0>(client) == name) {
            user = client;
            found = true;
            break;
        }
    }
    if (!found) {
        std::cout << "Could not authenticate the user.\n";
        return std::make_tuple("", 0, std::vector<std::string>());
    }
    return user;
}

bool checkPermissions(std::string sender, std::string receiver) {
    auto user = getUserByName(sender);

    bool hasPermission = false;
    auto permissions = std::get<2>(user);
    for (const auto &permission : permissions) {
        if (permission == receiver) {
            hasPermission = true;
            break;
        }
    }

    return hasPermission;
}

void handleRegister(std::string name, int socket) {
    // Add client to the registry
    clients.push_back(std::make_tuple(name, socket, std::vector<std::string>{name, "user2"}));

    std::cout << "Registered the client: " << name << "\n";

    // DEBUG
    // display clients

    std::cout << "Current client list:\n";
    for (const auto &client : clients) {
        std::cout << "Name: " << std::get<0>(client)
                  << "\n Socket: " << std::get<1>(client)
                  << "\n Permissions: ";

        for (const auto &str : std::get<2>(client)) {
            std::cout << str << " ";
        }
        std::cout << std::endl;
    }
}

void handleCommand(std::string command, std::string target_name, int socket) {
    // Get user
    auto user = getUserBySocket(socket);
    auto target = getUserByName(target_name);

    std::cout << "Parsing command from " << std::get<0>(user) << "\n";

    // Check permissions
    bool hasPermission = checkPermissions(std::get<0>(user), target_name);
    if (!hasPermission) {
        std::string res = "~You don't have sufficient permissions.";
        send(socket, res.c_str(), res.length(), 0);
        return;
    }
    // Send command to the target user
    send(std::get<1>(target), command.c_str(), command.length(), 0);
}

void parseRequest(char *request, int socket) {
    // Convert request to params
    std::string action, param1, param2;
    std::istringstream iss(request);
    iss >> action >> param1 >> param2;

    if (action == "REGISTER")
        handleRegister(param1, socket);
    if (action == "COMMAND")
        handleCommand(param1, param2, socket);

    // std::cout << "action: " << action << "\n";
    // std::cout << "param1: " << param1 << "\nparam2: " << param2 << "\n";
}

void *socketThread(void *arg) {
    int clientSocket = *((int *)arg);

    while (1) {
        // Await commands
        if (recv(clientSocket, client_message, sizeof(client_message), 0) == 0) {
            // Client closed the connection
            break;
        };
        std::cout << "message: " << client_message << "\n";

        if (strcmp(client_message, "COMMAND q") == 0) break;

        parseRequest(client_message, clientSocket);
        memset(&client_message, 0, sizeof(client_message));

        // printf("Received command: %s\n", client_message);

        // // Send command to the receiver
        // send(clientSocket, client_message, sizeof(client_message), 0);
    }

    std::cout << "Closing socket\n";
    if (close(clientSocket) != 0) std::cout << "Coudn't close the socket\n";
    pthread_exit(NULL);
}

int main() {
    setupSignalHandler();

    // Create the socket.
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        perror("Error creating socket");
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(55554);
    // serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Bind the address struct to the socket
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
        std::cout << "Couldn't bind " << errno << "\n";
        return 1;
    };
    std::cout << "Server running on port: " << ntohs(serverAddr.sin_port) << std::endl;

    if (listen(serverSocket, 10) == 0)
        printf("Listening\n");
    else
        printf("Error\n");

    pthread_t thread_id;

    while (1) {
        // Accept call creates a new socket for the incoming connection
        newSocket = accept(serverSocket, nullptr, nullptr);

        if (pthread_create(&thread_id, NULL, socketThread, &newSocket) != 0)
            printf("Failed to create thread\n");

        pthread_detach(thread_id);
        // pthread_join();?
    }
    return 0;
}