#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>

#include "server_user.h"

#define USER_FILE_EXTENTION "statistics"
#define USER_FILENAME_SIZE (USER_ID_SIZE + 12)
#define USER_FILE_MODE 0664

extern int check_id_validity(char* id);
extern void print_user_info(const UserInfo* user_info_p, FILE* fp);
extern int read_user_info_from_file(const char* id, UserInfo* user_info_buf);
extern int write_user_info_to_file(const UserInfo* user_info_p);
extern int print_user_info_file_list(FILE* fp);


int check_id_validity(char* id)
{
	if (!isalpha(*id))
		return 0;
	*id = tolower(*id);
	for (id = id + 1; *id; id++)
	{
		if (!isalnum(*id) && *id != '_')
			return 0;
		*id = tolower(*id);
	}
	return 1;
}

void print_user_info(const UserInfo* uip, FILE* fp)
{
	fprintf(fp, "%-10s  ", uip->id);
	fprintf(fp, "%5d  ", uip->money);
	fprintf(fp, "%4d  %4d  %4d  ",
		uip->win_count, uip->draw_count, uip->lose_count);
	if (uip->win_count > 0)
	{
		fprintf(fp, "    %6.2lf%%       ", (double)uip->win_count
			/ (uip->win_count + uip->draw_count + uip->lose_count) * 100.0);
	}
	else
	{
		fprintf(fp, "      0.00%%       ");
	}
	fprintf(fp, "%s", ctime(&uip->last_time));
}

static void get_user_info_filename(const char* id, char* filename_buf)
{
	strncpy(filename_buf, id, USER_ID_SIZE - 1);
	strcat(filename_buf, (const char*)("." USER_FILE_EXTENTION));
}


static int make_new_user_info_file(	const char* id, const char* filename, UserInfo* user_info_buf )
{
	int fd;

	/* Create a file */
	if ((fd = creat(filename, USER_FILE_MODE)) == -1)
	{
		perror(filename);
		return -1;
	}

	/* Initialize the user info */
	strncpy(user_info_buf->id, id, USER_ID_SIZE - 1);
	user_info_buf->money = 0;
	user_info_buf->win_count = 0;
	user_info_buf->lose_count = 0;
	user_info_buf->draw_count = 0;
	time(&user_info_buf->last_time);

	/* Write to the file */
	if (write(fd, user_info_buf, sizeof(UserInfo)) == -1)
	{
		perror(filename);
		close(fd);
		return -1;
	}

	/* Close the file descriptor */
	close(fd);
	return 0;
}


int read_user_info_from_file(const char* id, UserInfo* user_info_buf)
{
	int fd;
	char filename[USER_FILENAME_SIZE];

	if (!id || !*id)//null 체크
		return -1;

	/* Get the filename */
	get_user_info_filename(id, filename);

	/* Open the file */
	if ((fd = open(filename, O_RDONLY)) == -1)
	{
		/*perror(filename);*/
		/* If opening failed, try to make new one */
		if (make_new_user_info_file(id, filename, user_info_buf) == -1)
			return -1;
		return 0;
	}

	/* Read from the file */
	if (read(fd, user_info_buf, sizeof(UserInfo)) == -1)
	{
		perror(filename);
		close(fd);
		return -1;
	} 

	/* Refresh the UserInfo::last_time */
	time(&user_info_buf->last_time);

	/* Close the file descriptor */
	close(fd);
	return 0;
}


int write_user_info_to_file(const UserInfo* user_info_p)
{
	int fd;
	char filename[USER_FILENAME_SIZE];

	/* Get the filename */
	get_user_info_filename(user_info_p->id, filename);

	/* Open the file */
	if ((fd = open(filename, O_WRONLY | O_TRUNC)) == -1)
	{
		perror(filename);
		return -1;
	}

	/* Write to the file */
	if (write(fd, user_info_p, sizeof(UserInfo)) == -1)
	{
		perror(filename);
		close(fd);
		return -1;
	}
	
	/* Close the file descriptor */
	close(fd);
	return 0;
}

int print_user_info_file_list(FILE* fp)
{
	FILE* fp_ls;
	char buf[BUFSIZ];

	if ((fp_ls = popen("ls *." USER_FILE_EXTENTION, "r")) == NULL)
		return -1;

	while (fgets(buf, BUFSIZ - 1, fp_ls))
		fputs(buf, fp);
	
	pclose(fp_ls);

	return 0;
}