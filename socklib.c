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

static void print_all_addresses(struct hostent* hp, int portnum)
{
	char** cur;
	for (cur = hp->h_addr_list; *cur; cur++)
	{
		fprintf(stdout, "  Address: %s:%d\n", inet_ntoa(*(struct in_addr*)*cur), portnum);
	}
}

static int make_server_socket_q(int portnum, int backlog)
{
	int sock;
	struct sockaddr_in saddr;
	struct hostent* hp;
	char hostname[HOSTLEN];

	/* Step 1: get a socket */
	/* Step 2: build the address */
	/* Step 3: Bind the address to the socket */
	/* Step 4: Listen the socket */
	
	return sock;
}

int make_server_socket(int portnum)
{
	return make_server_socket_q(portnum, BACKLOG);
}

int connect_to_server(const char* host, int portnum)
{
	int sock;
	struct sockaddr_in serv_saddr;
	struct hostent* hp;

	/* Step 1: Get a socket */

	/* Step 2: Build the address */

	/* Step 3: Connect to server */

	return sock;
}

int set_socket_keep_alive(int fd)
{
	int optval = 1;
	return setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval));
}
