#include "model/models.h"
#include "http_server.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "./model/models.h"
#include "../util/errors.h"
#include <time.h>
#include <unistd.h>
#include "parser.h"
#include "../util/logger.h"


RequestModel from_request(const char* requestString) {
    RequestModel requestModel;
    memset(&requestModel, 0, sizeof(requestModel));
    if (!requestString) {
        return requestModel;
    }

    char* copy = strdup(requestString);
    if (!copy) {
        return requestModel;
    }
    char* save = NULL;

    // Parse the first line
    char* line = strtok_r(copy, "\n", &save);
    if (!line) {
        free(copy);
        return requestModel;
    }

    char method[16], resource[256], version[32];
    if (sscanf(line, "%15s %255s %31s", method, resource, version) != 3) {
        free(copy);
        return requestModel;
    }

    // Parse the method
    if (strcmp(method, "GET") == 0) requestModel.line.method = GET;
    else if (strcmp(method, "POST") == 0) requestModel.line.method = POST;
    else if (strcmp(method, "ECHO") == 0) requestModel.line.method = ECHO;
    else requestModel.line.method = UNKNOWN;

    strncpy(requestModel.line.resource, resource, sizeof(requestModel.line.resource) - 1);
    strncpy(requestModel.line.version,  version,  sizeof(requestModel.line.version)  - 1);

    // Parse the headers
    size_t headerCount = 0;
    char *bodyStart = NULL;

    while ((line = strtok_r(NULL, "\n", &save)) != NULL) {
        if (line[0] == '\0') {
            bodyStart = strtok_r(NULL, "", &save);
            break;
        }

        if (headerCount < MAX_HEADERS) {
            char* delim = strchr(line, ':');
            if (!delim) continue;

            *delim = '\0';
            char *name = line;
            char *value = delim + 1;

            while (*value == ' ' || *value == '\t') value++;

            Header *h = &requestModel.headers[headerCount++];
            strncpy(h->name,  name,  sizeof(h->name)  - 1);
            strncpy(h->value, value, sizeof(h->value) - 1);
        }
    }

    requestModel.headerCount = headerCount;

    // Parse the body
    if (bodyStart) {
        size_t len = strlen(bodyStart);
        if (len > 0) {
            requestModel.body = malloc(len + 1);
            if (requestModel.body) {
                memcpy(requestModel.body, bodyStart, len + 1);
                requestModel.bodySize = len;
            }
        }
    }

    free(copy);
    return requestModel;
}

ResponseModel to_response(int statusCode, const char* message, const char* body, size_t bodySize) {
    ResponseModel responseModel;
    memset(&responseModel, 0, sizeof(responseModel));

    strncpy(responseModel.status.version, "CHLP/1.0", sizeof(responseModel.status.version) - 1);
    responseModel.status.statusCode = statusCode;
    strncpy(responseModel.status.statusMsg, message,
            sizeof(responseModel.status.statusMsg) - 1);

    if (body && bodySize > 0) {
        responseModel.body = malloc(bodySize);
        if (responseModel.body) {
            memcpy(responseModel.body, body, bodySize);
            responseModel.bodySize = bodySize;
        }
    }

    char buf[32];
    snprintf(buf, sizeof(buf), "%zu", responseModel.bodySize);

    Header *header = &responseModel.headers[0];
    strncpy(header->name, "Body-Size", sizeof(header->name) - 1);
    strncpy(header->value, buf, sizeof(header->value) - 1);
    responseModel.headerCount = 1;

    return responseModel;
}

char* to_response_final(ResponseModel* responseModel) {
    char head[512];
    size_t offset = 0;

    int n = snprintf(head, sizeof(head), "%s %d %s\n", responseModel->status.version,
                     responseModel->status.statusCode, responseModel->status.statusMsg);
    if (n < 0 || (size_t)n >= sizeof(head)) {
        return NULL;
    }
    offset = (size_t)n;

    // Headers
    for (size_t i = 0; i < responseModel->headerCount; i++) {
        n = snprintf(head + offset, sizeof(head) - offset,
                     "%s: %s\n",
                     responseModel->headers[i].name,
                     responseModel->headers[i].value);
        if (n < 0 || (size_t)n >= sizeof(head) - offset) {
            return NULL;
        }
        offset += (size_t)n;
    }

    // Empty line
    if (offset + 2 >= sizeof(head)) return NULL;
    head[offset++] = '\n';
    head[offset] = '\0';

    size_t totalLen = offset + responseModel->bodySize;
    char *out = malloc(totalLen + 1);
    if (!out) return NULL;

    memcpy(out, head, offset);
    if (responseModel->bodySize > 0 && responseModel->body) {
        memcpy(out + offset, responseModel->body, responseModel->bodySize);
    }
    out[totalLen] = '\0';
    return out;

}

void print_request(RequestModel* requestModel) {
    printf("\n--------------------\n");
    printf("REQUEST:\n");
    printf("Request-Line: %s %s %s\n",
           method_to_str(requestModel->line.method),
           requestModel->line.resource,
           requestModel->line.version);

    printf("\nHeaders:\n");
    for (size_t i = 0; i < requestModel->headerCount; i++) {
        printf("  %s: %s\n",
               requestModel->headers[i].name,
               requestModel->headers[i].value);
    }

    if (requestModel->bodySize > 0 && requestModel->body) {
        printf("\nBody (%zu bytes):\n", requestModel->bodySize);
        fwrite(requestModel->body, 1, requestModel->bodySize, stdout);
        printf("\n");
    }

    printf("--------------------\n\n");
}

void print_response(ResponseModel* responseModel) {
    printf("\n--------------------\n");
    printf("RESPONSE:\n");
    printf("Status: %s %d %s\n",
           responseModel->status.version,
           responseModel->status.statusCode,
           responseModel->status.statusMsg);

    printf("\nHeaders:\n");
    for (size_t i = 0; i < responseModel->headerCount; i++) {
        printf("  %s: %s\n",
               responseModel->headers[i].name,
               responseModel->headers[i].value);
    }

    if (responseModel->bodySize > 0 && responseModel->body) {
        printf("\nBody (%zu bytes):\n", responseModel->bodySize);
        fwrite(responseModel->body, 1, responseModel->bodySize, stdout);
        printf("\n");
    }

    printf("--------------------\n\n");
}

void free_request(RequestModel* model) {
    if(model == NULL) {
        return;
    }

    free(model->body);
    model->body = NULL;
    model->bodySize = 0;
}

void free_response(ResponseModel* model) {
    if(model == NULL) {
        return;
    }

    free(model->body);
    model->body = NULL;
    model->bodySize = 0;
}

int handle_request(const RequestModel* request, ResponseModel* response) {
    if (!request || !response) return -1;

    log_info("Handling %s %s", method_to_str(request->line.method),
             request->line.resource);

    if (request->line.method == GET) {
        return handle_get(request, response);
    } else if (request->line.method == ECHO) {
        handle_echo(request, response);
        return 0;

    } else if (request->line.method == POST) {
        handle_post(request, response);
        return 0;
    }

    const char *msg = "Bad Request";
    *response = to_response(STATUS_BAD_REQUEST, "Bad Request",
                       msg, strlen(msg));
    return 0;
}

int handle_get(const RequestModel* request, ResponseModel* response) {
    // Find path to the resource
    const char *resource = request->line.resource;
    if (resource[0] == '/') resource++;

    char path[512];
    snprintf(path, sizeof(path), "assets/%s", resource);

    char* content = NULL;
    size_t size = 0;
    if (read_file_to_buffer(path, &content, &size) != 0) {
        const char *msg = "Not Found";
        *response = to_response(STATUS_NOT_FOUND, "Not Found", msg, strlen(msg));
        return 0;
    }

    *response = to_response(STATUS_OK, "OK", content, size);
    free(content);
    return 0;
}

void handle_post(const RequestModel* req, ResponseModel* res) {
    if (req->bodySize == 0 || req->body == NULL) {
        const char *msg = "No POST data provided";
        *res = to_response(
            STATUS_BAD_REQUEST,
            "Bad Request",
            msg,
            strlen(msg)
        );
        return;
    }

    log_info("POST body received (%zu bytes): %.*s",
             req->bodySize,
             (int)req->bodySize,
             req->body);

    char savedPath[512];

    if (save_post_body(req->body, req->bodySize, savedPath) != 0) {
        const char *msg = "Failed to save POST data";
        *res = to_response(
            STATUS_INTERNAL,
            "Internal Server Error",
            msg,
            strlen(msg)
        );
        return;
    }

    const char *reply = "POST data received successfully";
    *res = to_response(
        STATUS_OK,
        "OK",
        reply,
        strlen(reply)
    );
}

void handle_echo(const RequestModel* request, ResponseModel* response) {
    *response = to_response(STATUS_OK, "OK",
                           request->body, request->bodySize);
}

static void make_unique_filename(char *buf, size_t size) {
    time_t now = time(NULL);
    struct tm tm_now;
    localtime_r(&now, &tm_now);

    snprintf(buf, size,
             "assets/post_%04d-%02d-%02d_%02d-%02d-%02d_%d.txt",
             tm_now.tm_year + 1900,
             tm_now.tm_mon + 1,
             tm_now.tm_mday,
             tm_now.tm_hour,
             tm_now.tm_min,
             tm_now.tm_sec,
             (int)getpid());  
}

int save_post_body(const char *body, size_t size, char *savedPath) {
    char filename[512];
    make_unique_filename(filename, sizeof(filename));

    FILE *f = fopen(filename, "wb");
    if (!f) {
        return -1;
    }

    size_t written = fwrite(body, 1, size, f);
    fclose(f);

    if (written != size) {
        return -1;
    }

    strcpy(savedPath, filename);
    return 0;
}
