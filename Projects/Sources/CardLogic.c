#include "CardLogic.h"
#include "Define.h"
#include <stdlib.h>


// デッキの初期化
BOOL initDeck(DeckData* const deckDataP, int maxCardNumber, int maxCardType)
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
        cardP->priority = (1 == cardP->number) ? maxCardNumber + 1 : cardP->number;
    }

    return TRUE;
}

// デッキからカードを引く
BOOL drawDeck(DeckData* const deckDataP, Card* const cardP)
{
    return getCard(deckDataP->cards, deckDataP->cardCount--, cardP);
}

// カードをシャッフル
BOOL shuffleCard(Card* const cardsP, int maxCardCount)
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
BOOL getCard(Card* const cardsP, int maxCardCount, Card* const outputCardP)
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
    cardP->priority = 0;

    return TRUE;
}

// 大小の判定
static BOOL isSortLess(int src, int dst)
{
    return (src < dst) ? TRUE : FALSE;
}
static BOOL isSortGreater(int src, int dst)
{
    return (dst < src) ? TRUE : FALSE;
}

// カードをソート
BOOL sortCards(Card* const cardsP, int maxCardCount, SortOrder sortOrder)
{
    int base, i;
    Card card;

    if (!cardsP || maxCardCount == 0)
    {
        return FALSE;
    }

    for (base = 1; base < maxCardCount; ++base)
    {
        for (i = base; 0 < i; --i)
        {
            if (sortOrder(cardsP[i].priority, cardsP[i - 1].priority))
            {
                card = cardsP[i - 1];
                cardsP[i - 1] = cardsP[i];
                cardsP[i] = card;
            }
            else
            {
                break;
            }
        }
    }

    return TRUE;
}

// フラッシュが成立しているか
static BOOL isHandForFlash(Card* const cardsP, int maxCardCount)
{
    int i;
    BOOL isHand = TRUE;
    eCardType type;

    if (!cardsP || maxCardCount != PLAYER_MAX_CARD_CNT)
    {
        return FALSE;
    }

    type = cardsP[0].type;
    for (i = 1; i < maxCardCount; ++i)
    {
        isHand &= (type == cardsP[i].type);
    }

    return isHand;
}

// ストレートが成立しているか
static BOOL isHandForStraight(Card* const cardsP, int maxCardCount)
{
    int i;
    BOOL isHand = TRUE;

    if (!cardsP || maxCardCount != PLAYER_MAX_CARD_CNT)
    {
        return 0;
    }

    for (i = 1; i < maxCardCount; ++i)
    {
        isHand &= (cardsP[i - 1].priority == (cardsP[i].priority - 1));
    }

    return isHand;
}

// スリーカードが成立しているか
static BOOL isHandForThreeCard(Card* const cardsP, int maxCardCount)
{
    int i, count = 1;
    BOOL isHand = FALSE;

    if (!cardsP || maxCardCount != PLAYER_MAX_CARD_CNT)
    {
        return FALSE;
    }

    for (i = 1; i < maxCardCount; ++i)
    {
        if (cardsP[i - 1].number == cardsP[i].number)
        {
            count++;
        }
        else
        {
            (count == 3) && (isHand = TRUE);
            count = 1;
        }
    }

    // 最終チェックの結果を反映
    (count == 3) && (isHand = TRUE);

    return isHand;
}

// フォーカードが成立しているか
static BOOL isHandForFourCard(Card* const cardsP, int maxCardCount)
{
    int i, count = 1;
    BOOL isHand = FALSE;

    if (!cardsP || maxCardCount != PLAYER_MAX_CARD_CNT)
    {
        return FALSE;
    }

    for (i = 1; i < maxCardCount; ++i)
    {
        if (cardsP[i - 1].number == cardsP[i].number)
        {
            count++;
        }
        else
        {
            (count == 4) && (isHand = TRUE);
            count = 1;
        }
    }

    // 最終チェックの結果を反映
    (count == 4) && (isHand = TRUE);

    return isHand;
}

// ペア数を取得
static int getHandForPairCount(Card* const cardsP, int maxCardCount)
{
    int i, pairCount = 0, count = 1;

    if (!cardsP || maxCardCount != PLAYER_MAX_CARD_CNT)
    {
        return 0;
    }

    for (i = 1; i < maxCardCount; ++i)
    {
        if (cardsP[i - 1].number == cardsP[i].number)
        {
            count++;
        }
        else
        {
            (count == 2) && pairCount++;
            count = 1;
        }
    }

    // 最終チェックの結果を反映
    (count == 2) && pairCount++;

    return pairCount;
}

// カードの役を計算
BOOL calcPokerHand(Card* const cardsP, int maxCardCount, ePokerHand* const outputHandP)
{
    int pairCount;
    BOOL isFlash, isThreeCard, isFourCard, isStraight;
    ePokerHand hand = ePokerHandNil;

    if (!cardsP || !outputHandP || maxCardCount <= 0)
    {
        return FALSE;
    }

    // ソート
    if (!sortCards(cardsP, maxCardCount, isSortLess))
    {
        return FALSE;
    }

    // 役判定
    pairCount = getHandForPairCount(cardsP, maxCardCount);
    isFlash = isHandForFlash(cardsP, maxCardCount);
    isThreeCard = isHandForThreeCard(cardsP, maxCardCount);
    isFourCard = isHandForFourCard(cardsP, maxCardCount);
    isStraight = isHandForStraight(cardsP, maxCardCount);

    if (isFourCard)
    {
        // フォーカード
        hand = ePokerHandFourCards;
    }
    else if (isThreeCard)
    {
        // スリーカード
        // フルハウス
        hand = (0 < pairCount) ? ePokerHandFullHouse : ePokerHandThreeCards;
    }
    else if (isFlash)
    {
        // フラッシュ
        // ストレートフラッシュ
        // ロイヤルストレートフラッシュ
        if (isStraight)
        {
            hand = (cardsP[maxCardCount - 1].number == 1) ? ePokerHandRoyalStraightFlash : ePokerHandStraightFlash;
        }
        else
        {
            hand = ePokerHandFlash;
        }
    }
    else if (isStraight)
    {
        // ストレート
        hand = ePokerHandStraight;
    }
    else
    {
        // ワンペア
        // ツーペア
        if (pairCount == 1)
        {
            hand = ePokerHandOnePair;
        }
        else if (pairCount == 2)
        {
            hand = ePokerHandTwoPair;
        }
    }

    *outputHandP = hand;

    return TRUE;
}
