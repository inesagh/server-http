#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define CLIENT_COUNT 5

int create_bind_server(int port, struct sockaddr_in * server) {
    int serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if(serverFd == -1) {
        perror("Socket failed!");
        exit(EXIT_FAILURE);
    }

    memset(server, 0, sizeof(struct sockaddr_in));
    server->sin_family = AF_INET;
    server->sin_port = htons(port);
    server->sin_addr.s_addr = INADDR_ANY;
    socklen_t len = sizeof(*server);

    
    if(bind(serverFd, (struct sockaddr *) server, len) == -1) {
        perror("Bind failed!");
        close(serverFd);
        exit(EXIT_FAILURE);
    }

    if(listen(serverFd, CLIENT_COUNT) == -1){ 
        perror("Listen failed!");
        close(serverFd);
        exit(EXIT_FAILURE);
    }

    return serverFd;
}