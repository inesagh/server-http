#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <unistd.h>

#include "core/server.h"
#include "core/thread.h"
#include "util/logger.h"

#define SERVER_PORT 8080

int main(void) {
    struct sockaddr_in server;
    int serverFd = create_bind_server(SERVER_PORT, &server);

    log_info("Server listening on port %d", SERVER_PORT);

    while (1) {
        struct sockaddr_in client;
        socklen_t clientLen = sizeof(client);

        int clientFd = accept(serverFd, (struct sockaddr *)&client, &clientLen);
        if (clientFd < 0) {
            log_error("accept failed");
            continue;
        }

        log_info("Accepted new client with fd=%d\n", clientFd);
        if (start_thread(clientFd) != 0) {
            log_error("Failed to start client thread");
        }
    }

    close(serverFd);
    return 0;
}