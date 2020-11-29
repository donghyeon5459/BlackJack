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