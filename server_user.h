#include <time.h>
#define USER_ID_SIZE 16

#define USER_INFO_HEADER_STR \
"    ID     MONEY   WIN  DRAW  LOSE   " \
"WIN PERCENTAGE           LAST TIME"

typedef struct UserInfo
{
	char id[USER_ID_SIZE];
	int money;
	int win_count;
	int draw_count;
	int lose_count;
	time_t last_time;
}UserInfo;