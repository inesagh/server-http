#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <strings.h> 

#include "parser.h"
#include "../util/logger.h"
#include "../util/files.h"

static void trim_newline(char* line) {
    size_t len = strlen(line);
    while (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r')) {
        line[len-1] = '\0';
        len--;
    }
}

static HttpMethod str_to_method(const char* m) {
    if (strcmp(m, "GET") == 0) return GET;
    if (strcmp(m, "POST") == 0) return POST;
    if (strcmp(m, "ECHO") == 0) return ECHO;
    return UNKNOWN;
}

const char* method_to_str(HttpMethod m) {
    switch (m) {
        case GET:  return "GET";
        case POST: return "POST";
        case ECHO: return "ECHO";
        default:   return "UNKNOWN";
    }
}

int parse_request(int fd, RequestModel* request) {
    memset(request, 0, sizeof(RequestModel));
    request->body = NULL;
    request->bodySize = 0;
    request->headerCount = 0;

    char lineBuf[512];

    ssize_t n = read_line(fd, lineBuf, sizeof(lineBuf));
    if (n <= 0) {
        log_error("Failed to read request line");
        return -1;
    }
    trim_newline(lineBuf);

    char methodStr[32], resource[256], version[32];
    if (sscanf(lineBuf, "%31s %255s %31s", methodStr, resource, version) != 3) {
        log_error("Malformed request line");
        return -1;
    }

    request->line.method = str_to_method(methodStr);
    strncpy(request->line.resource, resource, MAX_RESOURCE_LEN - 1);
    request->line.resource[MAX_RESOURCE_LEN - 1] = '\0';

    strncpy(request->line.version,  version, MAX_VERSION_LEN - 1);
    request->line.version[MAX_VERSION_LEN - 1] = '\0';

    size_t bodySize = 0;

    while (1) {
        n = read_line(fd, lineBuf, sizeof(lineBuf));
        if (n < 0) {
            log_error("Header read failed");
            return -1;
        }

        if (n == 0 || strcmp(lineBuf, "\n") == 0 || strcmp(lineBuf, "\r\n") == 0) {
            break;
        }

        trim_newline(lineBuf);
        if (lineBuf[0] == '\0') {
            break;
        }

        if (request->headerCount >= MAX_HEADERS) {
            log_error("Too many headers");
            return -1;
        }

        Header *h = &request->headers[request->headerCount];
        if (parse_header_line(lineBuf, h) != 0) {
            log_error("Invalid header format: '%s'", lineBuf);
            return -1;
        }

        request->headerCount++;

        if (strcasecmp(h->name, "Body-Size") == 0) {
            bodySize = (size_t)strtoul(h->value, NULL, 10);
        }
    }

    request->bodySize = bodySize;
    if (bodySize > 0) {
        request->body = malloc(bodySize + 1);
        if (!request->body) {
            log_error("Failed to allocate memory for body");
            return -1;
        }

        if (read_from(fd, request->body, bodySize) < 0) {
            log_error("Failed to read body");
            free(request->body);
            request->body = NULL;
            request->bodySize = 0;
            return -1;
        }
        request->body[bodySize] = '\0';
    }

    return 0;
}

int parse_header_line(char* line, Header* h) {
    char* delim = strchr(line, ':');
    if (!delim) {
        log_error("Header without ':'");
        return -1;
    }

    *delim = '\0';
    char *name = line;
    char *value = delim + 1;

    while (*value == ' ' || *value == '\t') value++;

    size_t vlen = strlen(value);
    while (vlen > 0 && (value[vlen - 1] == '\r' || value[vlen - 1] == '\n')) {
        value[--vlen] = '\0';
    }

    strncpy(h->name,  name,  MAX_HEADER_NAME  - 1);
    h->name[MAX_HEADER_NAME - 1] = '\0';

    strncpy(h->value, value, MAX_HEADER_VALUE - 1);
    h->value[MAX_HEADER_VALUE - 1] = '\0';

    return 0;
}
