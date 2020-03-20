#include "CardLogic.h"
#include "Define.h"
#include <stdlib.h>


// デッキの初期化
BOOL initDeck(DeckData* const deckDataP, const int maxCardNumber, const int maxCardType)
{
    int i;
    Card* cardP;
    const int maxCardCount = maxCardNumber * maxCardType;

    if (!deckDataP)
    {
        return FALSE;
    }

    for (i = 0; i < maxCardCount; ++i)
    {
        cardP = &deckDataP->cards[i];
        cardP->type = (i / maxCardNumber) + 1;
        cardP->number = (i % maxCardNumber) + 1;
    }

    return TRUE;
}

// デッキからカードを引く
BOOL drawDeck(DeckData* const deckDataP, Card* const cardP)
{
    return getCard(deckDataP->cards, deckDataP->cardCount--, cardP);
}

// カードをシャッフル
BOOL shuffleCard(Card* const cardsP, const int maxCardCount)
{
    int i, cardNum;
    Card card;

    if (!cardsP)
    {
        return FALSE;
    }

    for (i = 0; i < maxCardCount; ++i)
    {
        cardNum = rand() % maxCardCount;
        card = cardsP[i];
        cardsP[i] = cardsP[cardNum];
        cardsP[cardNum] = card;
    }

    return TRUE;
}

// カードを1枚引く
BOOL getCard(Card* const cardsP, const int maxCardCount, Card* const outputCardP)
{
    Card* cardP;

    if (!cardsP || !outputCardP || maxCardCount <= 0)
    {
        return FALSE;
    }

    cardP = &cardsP[maxCardCount - 1];
    *outputCardP = *cardP;

    // 引いたカードを初期化
    cardP->type = eCardTypeNil;
    cardP->number = 0;

    return TRUE;
}

// カードの役を計算
BOOL calcPokerHand(Card* const cardsP, const int maxCardCount, ePokerHand* const outputHandP)
{
    int aceCount = 0, totalValue = 0;

    if (!cardsP || !outputHandP || maxCardCount <= 0)
    {
        return FALSE;
    }

    // 役判定
    *outputHandP = ePokerHandNil;

    return TRUE;
}
