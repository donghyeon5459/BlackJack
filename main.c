#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <errno.h>
#include <netdb.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include "server_user.h"


#define USER_WAITING_MAX 8  //대기방 최대 8명
#define ENTER_ROOM_BUF_SIZE 256
#define ESC_CHAR '\x1b'

#define USER_STATE_EMPTY 0
#define USER_STATE_WAITING 1
#define USER_STATE_READY 2
#define USER_STATE_ERROR 3

int main(void)
{
    prinf("taegyun commit test\n");
    return 0;
}