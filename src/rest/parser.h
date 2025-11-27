#ifndef PARSER_H
#define PARSER_H

#include <stddef.h>
#include "../util/files.h"
#include "model/models.h"

int parse_request(int, RequestModel*);
const char* method_to_str(HttpMethod);
int parse_header_line(char*, Header*);
#endif