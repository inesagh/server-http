#ifndef SERVER
#define SERVER

#include <netinet/in.h>

int create_bind_server(int, struct sockaddr_in *);

#endif
