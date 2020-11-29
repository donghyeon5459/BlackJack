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

static void sigchld_handler(int signum)
{
	/* It is called when a child process is dead. */
	pid_t pid;
	int status;

	while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
	{
		printf("A child process %d terminated", pid);
		if (WIFEXITED(status))
			printf(" (exit code %d).\n", WEXITSTATUS(status));
		else
			printf(" (by signal \"%s\").\n", strsignal(WTERMSIG(status)));
	}
}

static void sigint_handler(int signum)
{
	printf(" => Got SIGINT!\n");
}

int main(int argc, const char* argv[])
{
	int sock, fd, port; /* socket and connection */
	struct sigaction sigchld_act, sigint_act;
	pthread_t room_thread;

	if (argc != 2)
	{
		fprintf(stderr, "usage: %s [port number]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	/* Print the user file list */
	printf("User file list:\n");
	print_user_info_file_list(stdout);
	fputc('\n', stdout);

	/* Set the SIGCHLD signal handler */
	sigchld_act.sa_handler = sigchld_handler;
	sigchld_act.sa_flags = SA_RESTART;
	sigemptyset(&sigchld_act.sa_mask);
	sigaction(SIGCHLD, &sigchld_act, NULL);
	
	/* Set the SIGCHLD signal handler */
	sigint_act.sa_handler = sigint_handler;
	sigint_act.sa_flags = 0;	/* Eliminate SA_RESTART
								 * (Restarts the interrupted system calls) */
	sigfillset(&sigint_act.sa_mask);
	sigaction(SIGINT, &sigint_act, NULL);

	/* Make the server socket */
	port = atoi(argv[1]);
	if ((sock = make_server_socket(port)) == -1)
	{
		perror("server socket");
		fprintf(stderr, "Exiting the server...\n");
		exit(EXIT_FAILURE);
	}

	printf("Starting the waiting room thread...\n");

	/* Create the waiting room thread */
	if (pthread_create(
		&room_thread, NULL, waiting_room_routine, &index
		) == -1)
	{
		perror("Waiting room routine pthread_create");
		fprintf(stderr, "Exiting the server...\n");
		exit(EXIT_FAILURE);
	}

	printf("Starting the accept loop...\n");

	/* Accept routine */
	while (1)
	{
		if ((fd = accept(sock, NULL, NULL)) == -1)
		{
			/* It is fine because SIGCHLD's flag is set to SA_RESTART 
			 * If not set, must check whether interrupted by signal or not: */
			/*if (errno != EINTR)*/
			break;
		}
		process_request(fd);
	}
	
	/* Close the server socket */
	close(fd);

	/* Handle the waiting room thread */
	pthread_cancel(room_thread);
	pthread_join(room_thread, NULL);

	printf("This server(PID %d) is being closed...\n", getpid());
	printf("\nBut the child processes (except sh) may remain:\n");
	print_child_process_list();

	return 0;
}
