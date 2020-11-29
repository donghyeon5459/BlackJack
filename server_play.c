#include <stdio.h>
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