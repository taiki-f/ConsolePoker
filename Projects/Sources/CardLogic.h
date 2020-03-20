#ifndef __CARD_LOGIC_H__
#define __CARD_LOGIC_H__

#include "CardDefine.h"
#include "Define.h"


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
BOOL initDeck(DeckData* const deckDataP, const int maxCardNumber, const int maxCardType);
BOOL drawDeck(DeckData* const deckDataP, Card* const cardP);
BOOL shuffleCard(Card* const cardsP, const int maxCardCount);
BOOL getCard(Card* const cardsP, const int maxCardCount, Card* const outputCardP);
BOOL calcPokerHand(Card* const cardsP, const int maxCardCount, ePokerHand* const outputHandP);

#endif  // __CARD_LOGIC_H__
