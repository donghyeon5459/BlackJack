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


	
	if ((sock = socket(PF_INET, SOCK_STREAM, 0)) == -1)
		return -1;

	/* Step 2: build the address */
	gethostname(hostname, HOSTLEN);
	hp = gethostbyname(hostname);
	//hp = gethostbyname("155.230.28.224");

	fprintf(stdout, "Making the server...\n");
	print_all_addresses(hp, portnum);

	memset(&saddr, 0, sizeof(saddr));
	//memcpy(&saddr.sin_addr, hp->h_addr, hp->h_length);  local에서만 작동
	saddr.sin_port = htons(portnum);
	saddr.sin_family = AF_INET;
	//saddr.sin_addr.s_addr = inet_addr("155.230.28.224");   //연구실 서버 ip 원격용
	//inet_aton("155.230.28.211",&saddr.sin_addr);

	/* Step 3: Bind the address to the socket */
	if (bind(sock, (struct sockaddr*)&saddr, sizeof(saddr)) == -1)
		return -1;

	/* Step 4: Listen the socket */
	if (listen(sock, backlog) == -1)
		return -1;
	
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


	/* Step 1: Get a socket */
	if ((sock = socket(PF_INET, SOCK_STREAM, 0)) == -1)
		return -1;

	/* Step 2: Build the address */
	if (!(hp = gethostbyname(host)))
		return -1;

	fprintf(stdout, "Connecting to the server...\n");
	print_all_addresses(hp, portnum);

	memset(&serv_saddr, 0, sizeof(serv_saddr));
	memcpy(&serv_saddr.sin_addr, hp->h_addr, hp->h_length);
	serv_saddr.sin_port = htons(portnum);
	serv_saddr.sin_family = AF_INET;
	inet_aton("155.230.28.224",&serv_saddr.sin_addr);  //연구실 서버
	//serv_saddr.sin_addr.s_addr = inet_addr("155.230.28.224");

	/* Step 3: Connect to server */
	if (connect(sock, (struct sockaddr*)&serv_saddr, sizeof(serv_saddr)) == -1)
		return -1;

	return sock;
}

int set_socket_keep_alive(int fd)
{
	int optval = 1;
	return setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval));
}
