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