#ifndef __CARD_LOGIC_H__
#define __CARD_LOGIC_H__

#include "CardDefine.h"
#include "Define.h"


//
// Defines
//
typedef BOOL(*SortOrder)(int, int);     // ソートオーダー


//
// Structs
//
typedef struct {        // カード
    eCardType type;
    int number;
} Card;

typedef struct {        // デッキデータ
    Card cards[MAX_CARD_NUM * MAX_CARD_TYPE];  // デッキのカード
    int cardCount;                             // デッキの枚数
} DeckData;


//
// Functions
//
BOOL initDeck(DeckData* const deckDataP, int maxCardNumber, int maxCardType);
BOOL drawDeck(DeckData* const deckDataP, Card* const cardP);
BOOL shuffleCard(Card* const cardsP, int maxCardCount);
BOOL getCard(Card* const cardsP, int maxCardCount, Card* const outputCardP);
BOOL sortCards(Card* const cardsP, int maxCardCount, SortOrder sortOrder);
BOOL calcPokerHand(Card* const cardsP, int maxCardCount, ePokerHand* const outputHandP);


#endif  // __CARD_LOGIC_H__
