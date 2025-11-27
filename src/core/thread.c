#include "thread.h"

#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>

#include "rest/http_server.h"
#include "util/logger.h"
#include "rest/parser.h"

typedef struct {
    int clientFd;
} ClientInfo;

static void *client_thread_main(void* arg) {
    ClientInfo* client = (ClientInfo *)arg;
    int clientFd = client->clientFd;
    free(client);

    RequestModel req;
    memset(&req, 0, sizeof(req));
    if (parse_request(clientFd, &req) != 0) {
        log_error("Failed to parse request");
        close(clientFd);
        return NULL;
    }
    print_request(&req);

    ResponseModel res;
    memset(&res, 0, sizeof(res));

    if (handle_request(&req, &res) != 0) {
        log_error("handle_request failed");
        free_request(&req);
        close(clientFd);
        return NULL;
    }

    char* responseString = to_response_final(&res);
    if (responseString) {
        send(clientFd, responseString, strlen(responseString), 0);
        free(responseString);
    }
    print_response(&res);

    free_request(&req);
    free_response(&res);
    close(clientFd);

    log_info("Client connection closed");
    return NULL;
}

int start_thread(int clientFd) {
    ClientInfo* client = malloc(sizeof(ClientInfo));
    if (!client) {
        close(clientFd);
        return -1;
    }
    client->clientFd = clientFd;

    pthread_t threadId;
    if (pthread_create(&threadId, NULL, client_thread_main, client) != 0) {
        log_error("pthread_create failed");
        close(clientFd);
        free(client);
        return -1;
    }

    pthread_detach(threadId);
    return 0;
}
