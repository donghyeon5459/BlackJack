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



int main() {
    printf("hello\n");

    /* 
    To do list:
    1. back up the tty mode
    2. no canonical mode, no delay mode for stdin
    3. connect to the server
    4. set no canonical and no delay mode for socket
    5. interaction start
    6. resotre tty mode
    7. exit
    */

   stdin_tty_mode(0, 0); // backup stdin tty_mode

}