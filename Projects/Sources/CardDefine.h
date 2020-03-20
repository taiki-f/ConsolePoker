#ifndef __CARD_DEFINE_H__
#define __CARD_DEFINE_H__

//
// Defines
//
typedef enum {
    eCardTypeNil,
    eCardTypeSpade,         // スペード
    eCardTypeClover,        // クローバー
    eCardTypeHeart,         // ハート
    eCardTypeDiamond,       // ダイヤ
} eCardType;
#define MAX_CARD_TYPE           (4)     // カードタイプの最大数
#define MAX_CARD_NUM            (13)    // カードの最大の数字
#define PLAYER_MAX_CARD_CNT     (5)     // プレイヤーの所持できる最大カード枚数
#define MAX_CHANGE_HAND_CNT     (2)     // 手札を交換できる回数

typedef enum {
    ePokerHandNil,
    ePokerHandOnePair,              // ワンペア
    ePokerHandTwoPair,              // ツーペア
    ePokerHandThreeCards,           // スリーカード
    ePokerHandStraight,             // ストレート
    ePokerHandFlash,                // フラッシュ
    ePokerHandFullHouse,            // フルハウス
    ePokerHandFourCards,            // フォーカード
    ePokerHandStraightFlash,        // ストレートフラッシュ
    ePokerHandRoyalStraightFlash,   // ロイヤルストレートフラッシュ
}ePokerHand;

#endif  // __CARD_DEFINE_H__
