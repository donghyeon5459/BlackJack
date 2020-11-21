#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>

#define HOSTLEN 256
#define BACKLOG 1


extern int make_server_socket(int portnum);
extern int connect_to_server(const char* host, int portnum);
extern int set_socket_keep_alive(int fd);