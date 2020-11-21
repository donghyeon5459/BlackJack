#include <stdio.h>
#include "server_user.h"

#define BLACK_JACK -2
#define BUST -3
#define DRAW 2
#define WIN 1
#define DRAW 2
#define LOSE 0

// 게임 진행 관련 구조체
typedef struct
{
    int deck[10];
    int cardN;
    int dividend;
    int index;
    int sum;
} user_deck;

int Calculate_Deck(user_deck *user) {
    int i;
    int sum = 0;
    int check = 0;

    // 관련 함수 구현 해야함 !!
}

// 카드 수, 딜러와 유저 변수
int card_deck[52];
user_deck dealer;
user_deck user[3];

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