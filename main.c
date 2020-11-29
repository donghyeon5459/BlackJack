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

static void clear_active_users()
{
	int i;
	for (i = 0; i < USER_WAITING_MAX; i++)
		user_states[i] = USER_STATE_EMPTY;
}

static void check_esc_in_buf(char* str)
{
	char* cur;
	for (cur = str; *cur; cur++)
	{
		if (*cur == ESC_CHAR)
		{
			strcpy(str, (const char*)"(esc)");
			break;
		}
	}
}

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

static void send_all_users_info()
{
	int i, j;
	for (i = 0; i < user_n; i++)
	{
		fprintf(user_fps[i], "\n%s\n", USER_INFO_HEADER_STR);
		for (j = 0; j < user_n; j++)
		{
			print_user_info(&user_info[j], user_fps[i]);
			fputc('\n', user_fps[i]);
		}
		fputc('\n', user_fps[i]);
		fflush(user_fps[i]);
	}
}

static void enclosing_game_routine()
{
	int i;

	printf("The game on process %d is held.\n", getpid());

	send_all_users_info();

	/* Call the main gaming room routine function here:
	 * use user_info, user_fps, and user_n
	 */
	start_game_routine();

	/* Write user info to files */
	for (i = 0; i < user_n; i++)
		write_user_info_to_file(&user_info[i]);
	
	/* Write a bye to each user */
	for (i = 0; i < user_n; i++)
	{
		fprintf(user_fps[i], "To %s: The game is over. "
			"Thank you for playing.\n", user_info[i].id);
		fflush(user_fps[i]);
	}
	
	send_all_users_info();

	printf("The game on process %d is done.\n", getpid());
}


static void* enter_waiting_room(void* argset_p)
{
	int index = *((int*)argset_p);
	FILE* fp = user_fps[index];
	char buf[ENTER_ROOM_BUF_SIZE];

	/* Send a signal to the room */
	pthread_mutex_lock(&room_lock);
	pthread_cond_signal(&room_cond);
	pthread_mutex_unlock(&room_lock);
	
	fprintf(fp, "Welcome to the Black Jack server!\n");
	fprintf(fp, "Type the ID [%d characters at most]: ", USER_ID_SIZE - 1);
	fflush(fp);

	/* Clear the buffer */
	memset(buf, 0, ENTER_ROOM_BUF_SIZE);

	/* Read id from the client */
	if (fgets(buf, ENTER_ROOM_BUF_SIZE - 1, fp) == NULL)
	{
		fprintf(stderr, "Client %d disconnected while reading the ID.\n",
			index);
		fclose(fp);
		/* Set user state to error */
		pthread_mutex_lock(&user_lock);
		user_states[index] = USER_STATE_ERROR;
		pthread_mutex_unlock(&user_lock);
		/* Send a signal to the room */
		pthread_mutex_lock(&room_lock);
		pthread_cond_signal(&room_cond);
		pthread_mutex_unlock(&room_lock);
		return NULL;
	}

	/* Get rid of a newline from the buffer */
	buf[strcspn(buf, "\r\n")] = '\0';
	buf[strcspn(buf, "\n")] = '\0';

	/* Check ID */
	if (!check_id_validity(buf))
	{
		check_esc_in_buf(buf);
		fprintf(stderr, "Client %d failed to log in as \"%s\" (invalid ID).\n",
			index, buf);
		/* Write a fail message */
		fprintf(fp, "To %s: Login failed due to invalid ID.\n", buf);
		fprintf(fp, "  ID must consist of alphabets, numbers, and underbars.");
		fprintf(fp, "\n  The first character must be an alphabet.\n");
		fflush(fp);
		fclose(fp);
		/* Set user state to error */
		pthread_mutex_lock(&user_lock);
		user_states[index] = USER_STATE_ERROR;
		pthread_mutex_unlock(&user_lock);
		/* Send a signal to the room */
		pthread_mutex_lock(&room_lock);
		pthread_cond_signal(&room_cond);
		pthread_mutex_unlock(&room_lock);
		return NULL;
	}

	/* Read the user info from the file */	
	if (read_user_info_from_file(buf, &user_info[index]) == -1)
	{
		fprintf(stderr, "Client %d failed to log in as \"%s\" (file read).\n",
			index, buf);
		/* Write a fail message */
		fprintf(fp, "To %s: Login failed due to the file system.\n", buf);
		fflush(fp);
		fclose(fp);
		/* Set user state to error */
		pthread_mutex_lock(&user_lock);
		user_states[index] = USER_STATE_ERROR;
		pthread_mutex_unlock(&user_lock);
		/* Send a signal to the room */
		pthread_mutex_lock(&room_lock);
		pthread_cond_signal(&room_cond);
		pthread_mutex_unlock(&room_lock);
		return NULL;
	}

	/* Write the login success message */
	fprintf(stdout, "Client %d logged in as %s.\n", index, buf);
	fprintf(fp, "To %s: Login success.\n", user_info[index].id);
	fflush(fp);

	/* Write the asking to get ready message */
	fprintf(fp, "To %s: Get ready? [y/n]: ", user_info[index].id);
	fflush(fp);

	/* Clear the buffer */
	memset(buf, 0, ENTER_ROOM_BUF_SIZE);

	/* Read a ready message */
	if (fgets(buf, ENTER_ROOM_BUF_SIZE - 1, fp) == NULL)
	{
		fprintf(stderr, "Client %d(%s) disconnected while waiting ready msg.\n",
			index, user_info[index].id);
		fclose(fp);
		/* Set user state to error */
		pthread_mutex_lock(&user_lock);
		user_states[index] = USER_STATE_ERROR;
		pthread_mutex_unlock(&user_lock);
		/* Send a signal to the room */
		pthread_mutex_lock(&room_lock);
		pthread_cond_signal(&room_cond);
		pthread_mutex_unlock(&room_lock);
		return NULL;
	}

	/* Get rid of a newline from the buffer */
	buf[strcspn(buf, "\r\n")] = '\0';
	buf[strcspn(buf, "\n")] = '\0';

	/* Error if the ready message is invalid */
	if (strcmp(buf, "Y") != 0 && strcmp(buf, "y") != 0)
	{
		check_esc_in_buf(buf);
		fprintf(stderr, "Client %d(%s) sent \"%s\" while waiting ready [Y].\n",
			index, user_info[index].id, buf);
		/* Write a fail message */
		fprintf(fp, "To %s: Getting ready failed!\n", user_info[index].id);
		fflush(fp);
		fclose(fp);
		/* Set user state to error */
		pthread_mutex_lock(&user_lock);
		user_states[index] = USER_STATE_ERROR;
		pthread_mutex_unlock(&user_lock);
		/* Send a signal to the room */
		pthread_mutex_lock(&room_lock);
		pthread_cond_signal(&room_cond);
		pthread_mutex_unlock(&room_lock);
		return NULL;
	}

	/* Write a message */
	fprintf(fp, "To %s: Waiting for other clients...\n\n", user_info[index].id);
	fflush(fp);

	/* Get ready */
	pthread_mutex_lock(&user_lock);
	user_states[index] = USER_STATE_READY;
	pthread_mutex_unlock(&user_lock);

	/* Send a signal to the room */
	pthread_mutex_lock(&room_lock);
	pthread_cond_signal(&room_cond);
	pthread_mutex_unlock(&room_lock);

	return NULL;
}

static void* waiting_room_routine(void* argset_p)
{
	int user_indexes[USER_WAITING_MAX];
	UserInfo user_info_in_order[USER_WAITING_MAX];
	FILE* user_fps_in_order[USER_WAITING_MAX];
	int i, j, not_all_ready, loop_enable = 1;

	/* Lock the room */
	pthread_mutex_lock(&room_lock);

	while (loop_enable)
	{
		/* Wait the condition signal */
		pthread_cond_wait(&room_cond, &room_lock);

		/* Lock the users */
		pthread_mutex_lock(&user_lock);

		/* Check if all ready */
		user_n = 0;
		not_all_ready = 0;
		for (i = 0; i < USER_WAITING_MAX; i++)
		{
			switch (user_states[i])
			{
			case USER_STATE_READY:
				/* Print all active users */
				for (j = 0; j < USER_WAITING_MAX; j++)
				{
					switch (user_states[j])
					{
					case USER_STATE_READY:
						fprintf(user_fps[i], "%-15s READY\n", user_info[j].id);
						break;
					case USER_STATE_WAITING:
						fprintf(user_fps[i], "PENDING...\n");
					}
				}
				fputc('\n', user_fps[i]);
				fflush(user_fps[i]);
				/* Memorize the index */
				user_indexes[user_n++] = i;
				break;
			case USER_STATE_WAITING:
				/* Set not all ready */
				not_all_ready = 1;
				break;
			case USER_STATE_ERROR:
				/* Join the thread in error */
				pthread_join(user_threads[i], NULL);
				user_states[i] = USER_STATE_EMPTY;
			}
		}

		/* Continue if all ready */
		if (not_all_ready || user_n <= 0)
		{
			/* Must unlock */
			pthread_mutex_unlock(&user_lock);
			pthread_mutex_unlock(&room_lock);
			continue;
		}

		printf("All ready for %d client%s.\n",
			user_n, (user_n == 1) ? "" : "s");

		/* Rearrange the users with user_n and join threads */
		for (i = 0; i < user_n; i++)
		{
			user_info_in_order[i] = user_info[user_indexes[i]];
			user_fps_in_order[i] = user_fps[user_indexes[i]];
			pthread_join(user_threads[user_indexes[i]], NULL);
		}
		memcpy(user_info, user_info_in_order, user_n * sizeof(UserInfo));
		memcpy(user_fps, user_fps_in_order, user_n * sizeof(FILE*));

		/* Fork to start a game */
		switch (fork())
		{
		default:
			/* Parent */
			clear_active_users();
			break;
		case 0:
			/* Child */
			/* Ignore SIGINT */
			signal(SIGINT, SIG_IGN);
			/* Start the game routine */
			enclosing_game_routine();
			/* End loop */
			loop_enable = 0;
			break;
		case -1:
			/* Fail */
			perror("fork in waiting room");
			/* Write an error message to each user */
			for (i = 0; i < user_n; i++)
			{
				fprintf(user_fps[i], "To %s: Failed to make a game room.\n",
					user_info[i].id);
				fflush(user_fps[i]);
			}
			clear_active_users();
		}

		/* Disconnect users */
		for (i = 0; i < user_n; i++)
			fclose(user_fps[i]);

		/* Must unlock */
		pthread_mutex_unlock(&user_lock);
	}

	/* Must unlock */
	pthread_mutex_unlock(&room_lock);

	return NULL;
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


static void process_request(int fd)
{
	FILE* fp;
	static int index;
	int found;

	if (!(fp = fdopen(fd, "r+")))
	{
		perror("Process request fdopen");
		fprintf(stderr, "Sorry, cannot open the socket in C FILE stream.\n");
		close(fd);
		return;
	}

	{
		/* Lock the mutex */
		pthread_mutex_lock(&user_lock);

		/* Check there is empty user slot */
		found = 0;
		for (index = 0; index < USER_WAITING_MAX; index++)
		{
			if (user_states[index] == USER_STATE_EMPTY)
			{
				found = 1;
				break;
			}
		}

		/* Return if the waiting room is full */
		if (!found)
		{
			fprintf(fp, "Sorry. The room is full!\n");
			fflush(fp);
			fclose(fp);
			pthread_mutex_unlock(&user_lock); /* Must unlock */
			return;
		}

		/* Turn on user_states and set fp */
		user_states[index] = USER_STATE_WAITING;
		user_fps[index] = fp;

		/* Unlock the mutex */
		pthread_mutex_unlock(&user_lock);
	}

	/* Create a thread */
	if (pthread_create(
		&user_threads[index], NULL, enter_waiting_room, &index
		) == -1)
	{
		perror("Entering waiting room pthread_create");
		fprintf(fp, "Sorry, entering waiting room failed\n");
		fflush(fp);
		fclose(fp);
	}
}
