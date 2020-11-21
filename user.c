#include <stdio.h>
#include <termios.h>
#include <fcntl.h>

void tty_mode(int fd, int how) {
    static struct termios original_mode;
    static int original_flags;

    if (!how) { // save original termios and flags
        tcgetattr(fd, &original_mode);
        original_flags = fcntl(fd, F_GETFL);
        return;
    }
    else { // restore original termios and flags
        tcsetattr(fd, TCSANOW, &original_mode);
        fcntl(fd, F_SETFL, original_flags);
    }
}

void nodelay_mode(int fd) {
    int flag = fcntl(fd, F_GETFL);
    flag |= O_NDELAY;
    fcntl(fd, F_SETFL, flag);
}

int main() {
    printf("hello\n");

    /* 
    To do list:
    1. back up the tty mode
    2. set no delay mode for stdin
    3. connect to the server
    4. set no delay mode for socket
    5. interaction start
    6. resotre tty mode
    7. exit
    */

   stdin_tty_mode(0, 0); // backup stdin tty_mode
   nodelay_mode(0); // set nodelay mode in stdin

}