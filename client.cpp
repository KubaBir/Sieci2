#include <arpa/inet.h>
#include <fcntl.h>  // for open
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
// #include <string.h>
#include <sys/socket.h>
#include <unistd.h>  // for close

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

int clientSocket;
bool con_closed = false;
pthread_t thread_id;  // Listener thread

void sigintHandler(int signum) {
    std::cout << "\nCTRL-C detected. Closing socket and exiting...\n";
    if (close(clientSocket) != 0) std::cout << "Couldn't close the client socket\n";
    exit(signum);
}

void setupSignalHandler() {
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = sigintHandler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;

    sigaction(SIGINT, &sigIntHandler, NULL);
}

void *socketThread(void *arg) {
    int newSocket = *((int *)arg);

    // Listen for a command
    char command[256];
    while (1) {
        if (recv(newSocket, command, sizeof(command), 0) == 0) {
            std::cout << "Connection closed.\n"
                      << "Press enter to quit\n";
            con_closed = true;
            break;
        };

        if (command[0] == '~') {
            // Handle message (messages start with '~')
            std::cout << command << "\n";
        } else {
            // Handle command
            if (std::strlen(command) > 0) printf("Executing: %s\n", command);
            if (system(command) != 0) std::cout << "Command execution failed!\n";
        }
        memset(&command, 0, sizeof(command));
    }

    close(newSocket);
    pthread_exit(NULL);
}

void registerClient(int socket, std::string name) {
    std::string message = "REGISTER " + name;
    if (send(socket, message.c_str(), message.length(), 0) < 0) {
        printf("Send failed\n");
    }
}

int main(int argc, char *argv[]) {
    setupSignalHandler();
    std::string message;
    sockaddr_in serverAddr;

    // Create the socket.
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    // Configure settings of the server address
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(55554);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Connect the socket to the server
    if (connect(clientSocket, (sockaddr *)&serverAddr, sizeof(serverAddr))) {
        std::cout << "Failed to connect " << errno << "\n";
        return 1;
    }

    // Register the client
    if (argc < 2) {
        std::cout << "Coudn't find the client name.\nUsage: ./client <name>\n";
        return -1;
    }
    registerClient(clientSocket, argv[1]);

    // Create listener thread
    if (pthread_create(&thread_id, NULL, socketThread, &clientSocket) != 0) {
        printf("Failed to create thread\n");
        return 1;
    }

    std::cout << "Send 'q' to exit\n";
    std::cout << "Type the command you want to send:\n";

    while (1) {
        // Send command
        std::getline(std::cin, message);
        if (!message.length()) continue;

        // message - <client_name> <command>
        message = "COMMAND " + message;
        if (send(clientSocket, message.c_str(), message.length(), 0) < 0) {
            if (!con_closed)
                printf("Send failed\n");
            return 1;
        }

        // Exit on 'q'
        if (message == "q") {
            std::cout << "Exiting...\n";
            // The socket will be closed once the server closes its socket
            break;
        }
    }

    // Await thread execution to display the command effects
    pthread_join(thread_id, NULL);

    return 0;
}