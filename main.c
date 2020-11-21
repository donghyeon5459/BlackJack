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


UserInfo user_info[USER_WAITING_MAX];
FILE* user_fps[USER_WAITING_MAX];
int user_n;


static int user_states[USER_WAITING_MAX] = { 0 };
static pthread_t user_threads[USER_WAITING_MAX];

static pthread_mutex_t user_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t room_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t room_cond = PTHREAD_COND_INITIALIZER;



/* From socklib */
int make_server_socket(int portnum);
/* From user */
int check_id_validity(char* id);
void print_user_info(const UserInfo* user_info_p, FILE* fp);
int print_user_info_file_list(FILE* fp);
int read_user_info_from_file(const char* id, UserInfo* user_info_buf);
int write_user_info_to_file(const UserInfo* user_info_p);
/* From play */
int start_game_routine();


int main(void)
{
    prinf("taegyun commit test\n");
    return 0;
}