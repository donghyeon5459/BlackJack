#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>

#define BUFSIZE 256 // user input buffer

int enable = 1;

// from socklib.c
int connect_to_server(const char* host, int portnum);

void tty_mode(int fd, int how) {
    static struct termios original_mode;
    static int original_flags;

    if (!how) {
        tcgetattr(fd, &original_mode);
        original_flags = fcntl(fd, F_GETFL);
    }
    else {
        tcsetattr(fd, TCSANOW, &original_mode);
        fcntl(fd, F_SETFL, original_flags);
    }
}

void nodelay_mode(int fd) {
    int flag = fcntl(fd, F_GETFL);
    flag |= O_NDELAY;
    fcntl(fd, F_SETFL, flag);
}

void interaction(int fd) {
    FILE *fp;
    char buf[BUFSIZE];
    char input;

    fp = fdopen(fd, "r+");

    while (enable) {
        // print server's output
        if (fgets(buf, BUFSIZE - 1, fp)) {
            printf("%s", buf);
        }
        // put user's input to socket
        if ((input = getchar()) != EOF) {
            fputc(input, fp);
        }
        usleep(10);
    }
}

void pipehandler(int signum) {
    printf("\nsignal : SIGPIPE\n");
    enable = 0;
}

int main(int argc, char *argv[]) {
   const char* server_address;
   int port_number, fd;

    // input server_address, port_number
   if (argc != 3) {
       fprintf(stderr, "Usage: %s <server address> <port number>\n", argv[0]);
       exit(1);
   }
    server_address = argv[1];
    port_number = atoi(argv[2]);

    signal(SIGPIPE, pipehandler); // if socket dies, handling sigpipe

    tty_mode(0, 0); // backup stdin tty_mode
    nodelay_mode(0); // set nodelay mode for stdin

    // connect to the server
    if ((fd = connect_to_server(server_address,port_number)) == -1) {
        fprintf(stderr, "Server connect error\n");
        exit(1);
    }

    // set nodelay mode for socket
    nodelay_mode(fd);

    // interaction with server
    interaction(fd);

    // close socket
    close(fd); 

    // restore stdin tty_mode
    tty_mode(0, 1); 

    printf("Closing the client...\n");
    return 0;
}