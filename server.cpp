#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
// gcc program.c -lpthread -Wall -o outfile Remember to link pthread bib

char client_message[256];
char buffer[256];

void *socketThread(void *arg) {
    int newSocket = *((int *)arg);
    int n;
    n = recv(newSocket, client_message, sizeof client_message, 0);
    printf("%s\n", client_message);

    send(newSocket, client_message, sizeof client_message, 0);
    memset(&client_message, 0, sizeof(client_message));

    pthread_exit(NULL);
}

int main() {
    int serverSocket, newSocket;
    struct sockaddr_in serverAddr;
    struct sockaddr_storage serverStorage;
    socklen_t addr_size;

    // Create the socket.
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(1100);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    // Bind the address struct to the socket
    bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr));

    if (listen(serverSocket, 10) == 0)
        printf("Listening\n");
    else
        printf("Error\n");

    pthread_t thread_id;

    while (1) {
        // Accept call creates a new socket for the incoming connection
        addr_size = sizeof serverStorage;
        newSocket = accept(serverSocket, (struct sockaddr *)&serverStorage,
                           &addr_size);

        if (pthread_create(&thread_id, NULL, socketThread, &newSocket) != 0)
            printf("Failed to create thread\n");

        pthread_detach(thread_id);
        // pthread_join();?
    }
    return 0;
}