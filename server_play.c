#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#include "server_user.h"


#define BLACK_JACK -2
#define BUST -3
#define DRAW 2
#define WIN 1
#define DRAW 2
#define LOSE 0

// 게임 진행 관련 구조체
typedef struct user_deck
{
    int deck[10];
    int cardN;
    int dividend;
    int index;
    int sum;
} user_deck;

extern int user_n;
extern FILE *user_fps[8];
extern UserInfo user_info[8];

// 카드 수, 딜러와 유저 변수
int card_deck[52];
user_deck dealer;
user_deck user[3];

// 카드 분배 함수
void Card_Distributor(user_deck *user, int n)
{
    static int cnt;
    int i;
    for (i = 0; i < n; i++)
    {
        user->deck[user->cardN] = card_deck[cnt++];
        user->cardN++;
    }
}

// 덱 계산 함수
int Calculate_Deck(user_deck *user)
{
    int i;
    int sum = 0;
    int check = 0;

    for (i = 0; i < user->cardN; i++)
    {
        if (user->deck[i] % 13 == 0)
        {
            sum += 11;
        }
        else if (0 < user->deck[i] % 13 && user->deck[i] % 13 < 10)
            sum += (user->deck[i] % 13) + 1;
        else
            sum += 10;
    }

    /*calcuate deck*/

    for (i = 0; i < user->cardN; i++)
    {
        if (user->deck[i] % 13 == 0 && sum > 21)
            sum -= 10;
    }

    if (sum > 21)
        return BUST;
    else if (sum == 21)
        return BLACK_JACK;
    else
        return sum;
}

// 셔플 함수
void Shuffle(int *index, int nMax)
{
    int i, n;
    int temp;

    srand(time(NULL));
    for (i = nMax - 1; i >= 0; i--)
    {
        n = rand() % nMax;
        temp = index[i];
        index[i] = index[n];
        index[n] = temp;
    }
}

// index 값들에게 카드 값들을 부여하는 함수
void IndexToString(int index, char *card)
{
    int num;
    char num2[5];

    if (index / 13 == 0)
        strcpy(card, "clover_");
    else if (index / 13 == 1)
        strcpy(card, "heart_");
    else if (index / 13 == 2)
        strcpy(card, "diamond_");
    else
        strcpy(card, "spade_");

    if (index % 13 == 0)
        strcat(card, "A");

    else if (0 < index % 13 && index % 13 < 10)
    {
        num = (index % 13) + 1;
        sprintf(num2, "%d", num);
        strcat(card, num2);
    }
    else if (index % 13 == 10)
        strcat(card, "J");
    else if (index % 13 == 11)
        strcat(card, "Q");
    else
        strcat(card, "K");
}

// 덱 print 함수
void print_Deck(user_deck *puser, FILE *fp)
{
    char s[20];
    int i = 0;

    for (i = 0; i < puser->cardN; i++)
    {
        IndexToString(puser->deck[i], s);
        fprintf(fp, "%s\n", s);
    }
    fprintf(fp, "\n\n");
}

void print_other_Deck(user_deck *puser, FILE *fp, int idx)
{
    char s[20];
    int i;

    fprintf(fp, "Deck of dealer:\n");
    IndexToString(dealer.deck[0], s);
    fprintf(fp, "%s\n\n", s);

    for (i = 0; i < user_n; i++)
    {
        if (idx == i)
            continue;
        fprintf(fp, "user:%s \n", user_info[i].id);
        print_Deck(&user[i], fp);
        fflush(fp);
    }
}

// 게임 세팅 함수 구현
void gameSetting(user_deck *user, int userN) {
    int i;

    // 카드 초기화
    for (i = 0; i < 52; i++)
        card_deck[i] = i;
    /*suffle the deck*/
    Shuffle(card_deck, 52);
    // 각자의 카드 정보와 값 초기화
    for (i = 0; i < userN; i++)
    {
        memset(user + i, 0, sizeof(user_deck));
        Card_Distributor(user + i, 2);
    }
    memset(&dealer, 0, sizeof(user_deck));
    while (Calculate_Deck(&dealer) <= 16 && Calculate_Deck(&dealer) != BLACK_JACK && Calculate_Deck(&dealer) != BUST)
    {
        Card_Distributor(&dealer, 1);
    }
}

// 블랙잭 게임 내부 구현
void *play_game(void *data)
{
    user_deck *user = data;
    FILE *fp = user_fps[user->index];
    char buf[20];
    int idx = user->index;
    char dividend[100];

    // 게임에 처음 참가하는 경우 1000 배당
    if (user_info[idx].draw_count + user_info[idx].win_count + user_info[idx].lose_count == 0)
    {
        fprintf(fp, "처음으로 접속하셨습니다! 첫 접속 기념으로  1000$를 지급해드리겠습니다.\n");
        user_info[idx].money += 1000;
    }

    // 배당금 설정
    while (1)
    {
        fprintf(fp, "배당금을 입력해주세요(100~10000): ");
        fflush(fp);
        fgets(buf, sizeof(buf), fp);
        user->dividend = atoi(buf);
        if (user->dividend < 100 || user->dividend > 10000)
        {
            fprintf(fp, "잘못된 범위의 금액을 입력하셨습니다.\n");
            fflush(fp);
            continue;
        }
        fflush(fp);

        if (user->dividend > user_info[idx].money)
        {
            fprintf(fp, "당신이 보유한 금액은 %d입니다. 더 적은 금액을 입력해주세요\n", user_info[idx].money);
            continue;
        }
        user_info[idx].money -= user->dividend;
        break;
    }
    fprintf(fp, "카드 두장을 나눠드리겠습니다. 당신이 받은 카드는 다음과 같습니다\n");
    print_Deck(user, fp);
    // 카드를 받자마자 경기가 끝나는 경우
    if (Calculate_Deck(user) == BUST)
    {
        fprintf(fp, "BUST입니다. 당신은 패배했습니다\n\n");
        fprintf(fp, "당신은 %d만큼의 배당금을 잃었습니다. 현재 보유 금액 :%d \n\n", user->dividend, user_info[idx].money);
        user_info[idx].lose_count++;
        fprintf(fp, "Deck of dealer:\n");
        print_Deck(&dealer, fp);
        fflush(fp);
        fprintf(fp, "다른 플레이어가 끝날 때까지 잠시만 기다려주세요..");
        fflush(fp);
        return NULL;
    }

    else if (Calculate_Deck(user) == BLACK_JACK && Calculate_Deck(&dealer) == BLACK_JACK)
    {
        fprintf(fp, "양측다 무승부입니다\n\n");
        user_info[idx].money += user->dividend;
        fprintf(fp, "배당금을 돌려드렸습니다. 현재 보유 금액 :%d \n\n", user_info[idx].money);
        user_info[idx].draw_count++;
        fprintf(fp, "Deck of dealer:\n");
        print_Deck(&dealer, fp);
        fflush(fp);
        fprintf(fp, "다른 플레이어가 끝날 때까지 잠시만 기다려주세요..");
        fflush(fp);
        return NULL;
    }
    else if (Calculate_Deck(user) != BLACK_JACK && Calculate_Deck(&dealer) == BLACK_JACK)
    {
        fprintf(fp, "딜러의 패가 블랙잭입니다. 당신은 패배했습니다\n\n");
        fprintf(fp, "당신은 %d만큼의 배당금을 잃었습니다. 현재 보유 금액 :%d \n\n", user->dividend, user_info[idx].money);
        user_info[idx].lose_count++;
        fprintf(fp, "Deck of dealer:\n");
        print_Deck(&dealer, fp);
        fflush(fp);
        fprintf(fp, "다른 플레이어가 끝날 때까지 잠시만 기다려주세요..");
        fflush(fp);
        return NULL;
    }

    // 그외
    while (1)
    {

        fprintf(fp, "Hit 하시려면 h를, Stand하시려면 s를, 다른 사람의 패를 보려면 p를눌러주세요(h/s/p)");
        fflush(fp);
        fgets(buf, sizeof(buf), fp);
        fflush(fp);
        fprintf(fp, "\n");

        if (buf[0] == 'h' || buf[0] == 'H')
        {
            Card_Distributor(user, 1);
            fprintf(fp, "카드를 1장 나눠드렸습니다.\n");
            print_Deck(user, fp);
            if (Calculate_Deck(user) == BUST && Calculate_Deck(&dealer) != BUST)
            {
                fprintf(fp, "BUST입니다. 당신은 패배했습니다\n\n");
                fprintf(fp, "당신은 %d만큼의 배당금을 잃었습니다. 현재 보유 금액 :%d \n\n", user->dividend, user_info[idx].money);
                user_info[idx].lose_count++;
                fprintf(fp, "Deck of dealer:\n");
                print_Deck(&dealer, fp);
                fflush(fp);
                fprintf(fp, "다른 플레이어가 끝날 때까지 잠시만 기다려주세요..");
                fflush(fp);
                return NULL;
            }
            else if (Calculate_Deck(user) == BUST && Calculate_Deck(&dealer) == BUST)
            {
                fprintf(fp, "무승부 입니다\n");
                user_info[idx].money += user->dividend;
                fprintf(fp, "현재 보유 금액 :%d \n", user_info[idx].money);
                user_info[idx].draw_count++;
                fflush(fp);
                fprintf(fp, "Deck of dealer:\n");
                print_Deck(&dealer, fp);
                fprintf(fp, "다른 플레이어가 끝날 때까지 잠시만 기다려주세요..");
                fflush(fp);
                return NULL;
            }

            continue;
        }

        else if (buf[0] == 'p' || buf[0] == 'P')
        {
            print_other_Deck(user, fp, idx);
            continue;
        }
        else if (buf[0] == 's' || buf[0] == 'S')
        {
            if (Calculate_Deck(user) > Calculate_Deck(&dealer))
            {
                fprintf(fp, "당신이 승리했습니다\n\n");
                user_info[idx].money += user->dividend * 2;
                fprintf(fp, "당신은 %d만큼의 배당금을 얻었습니다. 현재 보유 금액 :%d \n", user->dividend, user_info[idx].money);
                user_info[idx].win_count++;
            }
            else if (Calculate_Deck(user) < Calculate_Deck(&dealer))
            {
                fprintf(fp, "당신이 패배했습니다\n");
                fprintf(fp, "당신은 %d만큼의 배당금을 잃었습니다. 현재 보유 금액 :%d \n\n", user->dividend, user_info[idx].money);
                user_info[idx].lose_count++;
            }
            else
            {
                fprintf(fp, "무승부 입니다\n");
                user_info[idx].money += user->dividend;
                fprintf(fp, "현재 보유 금액 :%d \n", user_info[idx].money);
                user_info[idx].draw_count++;
            }
            fflush(fp);
            fprintf(fp, "Deck of dealer:\n");
            print_Deck(&dealer, fp);
            fprintf(fp, "다른 플레이어가 끝날 때까지 잠시만 기다려주세요..");
            fflush(fp);
            return NULL;
        }
        else
        {
            fprintf(fp, "잘못 입력하셨습니다(h/s)\n");
            continue;
        }
        break;

        print_other_Deck(user, fp, idx);
    }
}

void start_game_routine()
{
    int card_deck[52];
    char buf[50];
    int i;
    int dividend;

    pthread_t game_thread[8];
    gameSetting(&user[i], user_n);

    for (i = 0; i < user_n; i++)
    {
        user[i].index = i;
        pthread_create(&game_thread[i], NULL, play_game, &user[i]);
    }

    for (i = 0; i < user_n; i++)
        pthread_join(game_thread[i], NULL);
    printf("pthread_join is over\n");
    return;
}
