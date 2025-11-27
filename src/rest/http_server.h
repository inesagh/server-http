#ifndef HTTP
#define HTTP

#include "model/models.h"

RequestModel from_request(const char*);

ResponseModel to_response(int, const char*, const char*, size_t);
char* to_response_final(ResponseModel*);

void print_request(RequestModel*);
void print_response(ResponseModel*);

void free_request(RequestModel*);
void free_response(ResponseModel*);

int handle_request(const RequestModel*, ResponseModel*);
int handle_get(const RequestModel*, ResponseModel*);
void handle_post(const RequestModel*, ResponseModel*);
void handle_echo(const RequestModel*, ResponseModel*);

int save_post_body(const char *body, size_t size, char *savedPath);
#endif
