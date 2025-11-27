#ifndef MODELS_H
#define MODELS_H

#include <stddef.h>

#define MAX_METHOD_LEN      16
#define MAX_RESOURCE_LEN    256
#define MAX_VERSION_LEN     16
#define MAX_STATUS_MSG_LEN  64
#define MAX_HEADER_NAME     64
#define MAX_HEADER_VALUE    256
#define MAX_HEADERS         16

typedef enum {
    GET,
    POST,
    ECHO,
    UNKNOWN
} HttpMethod;

typedef struct {
    HttpMethod method;
    char resource[MAX_RESOURCE_LEN];
    char version[MAX_VERSION_LEN];
} RequestLine;

typedef struct {
    char version[MAX_VERSION_LEN];
    int statusCode;
    char statusMsg[MAX_STATUS_MSG_LEN];
} StatusLine;

typedef struct {
    char name[MAX_HEADER_NAME];
    char value[MAX_HEADER_VALUE];
} Header;

typedef struct {
    RequestLine line;
    Header headers[MAX_HEADERS];
    size_t headerCount;
    char *body;
    size_t bodySize;
} RequestModel;

typedef struct {
    StatusLine status;
    Header headers[MAX_HEADERS];
    size_t headerCount;
    char *body;
    size_t bodySize;
} ResponseModel;


#endif