#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <fcntl.h>

#define BUFSIZE 256 // user input buffer

// from socklib.c
int connect_to_server(const char* host, int portnum);

void tty_mode(int fd, int how) {
    static struct termios original_mode;
    static int original_flags;

    if (!how) { // save original termios and flags
        tcgetattr(fd, &original_mode);
        original_flags = fcntl(fd, F_GETFL);

        // printf("saved original tty_mode\n");
        return;
    }
    else { // restore original termios and flags
        tcsetattr(fd, TCSANOW, &original_mode);
        fcntl(fd, F_SETFL, original_flags);

        // printf("restored original tty_mode\n");
        return;
    }
}

void nodelay_mode(int fd) {
    int flag = fcntl(fd, F_GETFL);
    flag |= O_NDELAY;
    fcntl(fd, F_SETFL, flag);

    // printf("set nodelay_mode for fd: %d\n", fd);
    return;
}

int main(int argc, char *argv[]) {
    /* 
    To do list:
    1. back up the tty mode
    2. set no delay mode for stdin
    3. connect to the server
    4. set no delay mode for socket
    5. interaction start
    6. restore tty mode
    7. exit
    */
   const char* server_address;
   int port_number, fd;

    // input server_address, port_number
   if (argc != 3) {
       fprintf(stderr, "Usage: %s <server address> <port number>\n", argv[0]);
       exit(1);
   }
    server_address = argv[1];
    port_number = atoi(argv[2]);

    /*
    printf("server address: %s\n", server_address);
    printf("port_number :%d\n", port_number);
    */

    tty_mode(0, 0); // backup stdin tty_mode
    nodelay_mode(0); // set nodelay mode for stdin

    /* test user input
    char buf[BUFSIZE];
    while (1) {
        if (fgets(buf, BUFSIZE, stdin)) {
            printf("%s\n", buf);
        }
    }
    */ 

    /* 
    // connect to the server
    if ((fd = connect_to_server(port_number, fd)) == -1) {
        fprintf(stderr, "Server connect error\n");
        exit(1);
    }

    // set nodelay mode for socket
    nodelay_mode(fd);

    close(fd); // close socket
    */

    tty_mode(0, 1); // restore stdin tty_mode
    return 0;
}