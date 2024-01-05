#include <arpa/inet.h>
#include <fcntl.h>  // for open
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>  // for close

#include <cstdlib>
#include <iostream>

using namespace std;

void *socketThread(void *arg) {
    int newSocket = *((int *)arg);

    // Listen for a command
    char command[256];
    recv(newSocket, command, sizeof command, 0);

    printf("Executing: %s\n", command);
    if (system(command) != 0) cout << "Command execution failed!\n";

    pthread_exit(NULL);
}

int main() {
    char message[256];

    char buffer[256];
    int clientSocket;
    sockaddr_in serverAddr;
    socklen_t addr_size;

    // Create the socket.
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    // Configure settings of the server address
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(1100);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Connect the socket to the server using the address
    if (connect(clientSocket, (sockaddr *)&serverAddr, sizeof(serverAddr))) {
        cout << "Failed to connect\n";
        return 0;
    }

    // Create listener thread
    pthread_t thread_id;
    if (pthread_create(&thread_id, NULL, socketThread, &clientSocket) != 0)
        printf("Failed to create thread\n");
    // pthread_detach(thread_id);

    // Send command
    cout << "Type the command you want to send:\n";
    cin >> message;
    if (send(clientSocket, message, sizeof(message), 0) < 0) {
        printf("Send failed\n");
    }

    // Await thread execution to display the command effects
    pthread_join(thread_id, NULL);

    // bzero(buffer, sizeof(buffer));
    // // Read the message from the server into the buffer
    // if (recv(clientSocket, buffer, sizeof(buffer), 0) < 0) {
    //     printf("Receive failed\n");
    // }
    // // Print the received message
    // printf("Data received: %s\n", buffer);

    // memset(&message, 0, sizeof(message));

    // close(clientSocket);

    return 0;
}