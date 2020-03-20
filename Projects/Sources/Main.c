#include "CardLogic.h"
#include "Define.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>


//
// Macros
//
#if defined(_DEBUG)
#   define LOG(...) printf(__VA_ARGS__)
#   define CHK(f)   assert(f)
#else
#   define LOG(...) {}
#   define CHK(f)   f
#endif
#define PRINT(...)      printf(__VA_ARGS__)
#define PRINT_CLEAR()   system("cls")


//
// Defines
//
typedef enum {          // ゲームフェイズ
    eGamePhaseEnd,
    eGamePhasePlayer,           // プレイヤーフェイズ
    eGamePhaseResult,           // リザルトフェイズ
} eGamePhase;


//
// Structs
//
typedef struct {        // プレイデータ
    Card cards[PLAYER_MAX_CARD_CNT];        // 所持カード
    int cardCount;                          // 所持枚数
    ePokerHand hand;                        // ポーカーの役
} PlayData;

typedef eGamePhase(*PhaseExecute)();
typedef struct {        // フェイズ情報
    eGamePhase phase;                       // ゲームフェイズ
    PhaseExecute execute;                   // 実行関数
} PhaseInfo;


//
// Prototypes
//
static eGamePhase phasePlayer();
static eGamePhase phaseResult();


//
// Works
//
static const PhaseInfo PHASE_INFO[] = {     // フェイズ情報
    {eGamePhasePlayer, phasePlayer},
    {eGamePhaseResult, phaseResult},
    {eGamePhaseEnd, NULL},  // 終端判定
};
static eGamePhase m_gamePhase;              // ゲームフェイズ

static DeckData m_deck;                     // デッキデータ
static PlayData m_playerData;               // プレイヤーデータ
static int m_changeHandCount;               // 手札の交換回数


// 初期化
BOOL initialize()
{
    int i;
    const Card initCard = {
        .type = eCardTypeNil,
        .number = 0,
    };

    LOG("--- initialize\n");

    // 乱数のシート値を設定
    srand((unsigned int)time(NULL));

    // デッキの初期化
    m_deck.cardCount = MAX_CARD_NUM * MAX_CARD_TYPE;
    CHK(initDeck(&m_deck, MAX_CARD_NUM, MAX_CARD_TYPE));
    CHK(shuffleCard(m_deck.cards, m_deck.cardCount));

    // プレイヤーデータの初期化
    for (i = 0; i < PLAYER_MAX_CARD_CNT; ++i)
    {
        m_playerData.cards[i] = initCard;
    }

    // 初期枚数カードを引く
    for (i = 0; i < PLAYER_MAX_CARD_CNT; ++i)
    {
        CHK(drawDeck(&m_deck, &m_playerData.cards[i]));
    }
    m_playerData.cardCount = PLAYER_MAX_CARD_CNT;

    // 現在の役を計算しておく
    CHK(calcPokerHand(m_playerData.cards, m_playerData.cardCount, &m_playerData.hand));

    // プレイヤーフェイズから開始
    m_gamePhase = eGamePhasePlayer;

    return TRUE;
}

const char* const getPokerHandName(ePokerHand hand)
{
    typedef struct {
        ePokerHand hand;
        const char* const name;
    } HandInfo;
    const HandInfo handInfoList[] = {
        {.hand = ePokerHandOnePair,             .name = "ワンペア"},
        {.hand = ePokerHandTwoPair,             .name = "ツーペア"},
        {.hand = ePokerHandThreeCards,          .name = "スリーカード"},
        {.hand = ePokerHandStraight,            .name = "ストレート"},
        {.hand = ePokerHandFlash,               .name = "フラッシュ"},
        {.hand = ePokerHandFullHouse,           .name = "フルハウス"},
        {.hand = ePokerHandFourCards,           .name = "フォーカード"},
        {.hand = ePokerHandStraightFlash,       .name = "ストレートフラッシュ"},
        {.hand = ePokerHandRoyalStraightFlash,  .name = "ロイヤルストレートフラッシュ"},
        {.hand = ePokerHandNil,                 .name = "ハイカード"},
    };
    const HandInfo* handP = &handInfoList[0];

    while (handP->hand != ePokerHandNil)
    {
        if (handP->hand == hand)
        {
            break;
        }
        handP++;
    }

    return handP->name;
}

// プレイデータを表示
void printPlayData()
{
    int i;
    const PlayData* playDataP;

    {
        PRINT("■プレイヤー ");
        playDataP = &m_playerData;
        PRINT("[hand:%s]", getPokerHandName(playDataP->hand));
        for (i = 0; i < playDataP->cardCount; ++i)
        {
            PRINT("[%d:%2d]", i, playDataP->cards[i].number);
        }
        PRINT("\n");
    }
    PRINT("\n");
}

// プレイヤーフェイズ
static eGamePhase phasePlayer()
{
    //char drawSelectBuf[8];
    //char drawSelect;
    eGamePhase phase = eGamePhaseResult;//eGamePhasePlayer;

    LOG("--- phase player\n");
    printPlayData();

    //if ((BLACKJACK_VALUE == m_playerData.totalValue) ||
    //    (BLACKJACK_VALUE == m_dealerData.totalValue))
    //{
    //    // 既に終了しているのでリザルトフェイズへ
    //    phase = eGamePhaseResult;
    //}
    //else
    //{
    //    PRINT("> カードを引きますか？ (y/n) : ");
    //    scanf_s("%s", drawSelectBuf, (unsigned int)sizeof(drawSelectBuf));

    //    drawSelect = drawSelectBuf[0];
    //    if (drawSelect == 'y' || drawSelect == 'Y')
    //    {
    //        // カードを引く
    //        CHK(getCard(m_deckCards, m_deckCardCount--, &m_playerData.cards[m_playerData.cardCount]));
    //        m_playerData.cardCount++;
    //        CHK(calcTotalValue(m_playerData.cards, m_playerData.cardCount, &m_playerData.totalValue));

    //        if (BLACKJACK_VALUE <= m_playerData.totalValue)
    //        {
    //            // 21以上ならディーラーフェイズへ
    //            phase = eGamePhaseDealer;
    //        }
    //    }
    //    else if (drawSelect == 'n' || drawSelect == 'N')
    //    {
    //        // ディーラーフェイズへ
    //        phase = eGamePhaseDealer;
    //    }
    //    else
    //    {
    //        PRINT("入力が不正です。再度入力し直してください。\n");
    //    }
    //}

    return phase;
}

// リザルトフェイズ
static eGamePhase phaseResult()
{
    LOG("--- phase result\n");
    printPlayData();

    //if (m_playerData.totalValue == m_dealerData.totalValue)
    //{
    //    PRINT("△引き分けです\n");
    //}
    //else if (BLACKJACK_VALUE < m_dealerData.totalValue)
    //{
    //    PRINT("〇プレイヤーの勝利です\n");
    //}
    //else if (BLACKJACK_VALUE < m_playerData.totalValue)
    //{
    //    PRINT("×プレイヤーの敗北です\n");
    //}
    //else if (m_dealerData.totalValue < m_playerData.totalValue)
    //{
    //    PRINT("〇プレイヤーの勝利です\n");
    //}
    //else if (m_playerData.totalValue < m_dealerData.totalValue)
    //{
    //    PRINT("×プレイヤーの敗北です\n");
    //}
    //else
    //{
    //    assert(FALSE && "想定していない結果判定です");
    //}
    PRINT("\n\n");

    return eGamePhaseEnd;
}

// フェイズ実行関数を取得
static PhaseExecute getPhaseExecute(eGamePhase phase)
{
    const PhaseInfo* infoP = &PHASE_INFO[0];
    while (infoP->phase != eGamePhaseEnd)
    {
        if (infoP->phase == phase)
        {
            break;
        }
        infoP++;
    }

    return infoP->execute;
}

// フェイズの更新
BOOL updatePhase()
{
    BOOL isGameEnd = FALSE;
    const PhaseExecute execute = getPhaseExecute(m_gamePhase);
    if (execute)
    {
        m_gamePhase = execute();
    }
    else
    {
        isGameEnd = TRUE;
    }

    return isGameEnd;
}

// 更新
BOOL update()
{
    LOG("--- update\n");
    return updatePhase();;
}

// メイン関数
int main()
{
    char in[8] = "\0";
    BOOL isGameEnd = FALSE;

    while (in[0] != 'n' && in[0] != 'N')
    {
        PRINT_CLEAR();
        if (!initialize())
        {
            return -1;
        }

        while (!isGameEnd)
        {
            isGameEnd = update();
        }

        // ゲームをループできるようにする
        PRINT("> 再度やり直しますか？ (y/n) : ");
        scanf_s("%s", in, (unsigned int)sizeof(in));
        isGameEnd = FALSE;
    }

    return 0;
}

// プログラムの実行: Ctrl + F5 または [デバッグ] > [デバッグなしで開始] メニュー
// プログラムのデバッグ: F5 または [デバッグ] > [デバッグの開始] メニュー

// 作業を開始するためのヒント: 
//    1. ソリューション エクスプローラー ウィンドウを使用してファイルを追加/管理します 
//   2. チーム エクスプローラー ウィンドウを使用してソース管理に接続します
//   3. 出力ウィンドウを使用して、ビルド出力とその他のメッセージを表示します
//   4. エラー一覧ウィンドウを使用してエラーを表示します
//   5. [プロジェクト] > [新しい項目の追加] と移動して新しいコード ファイルを作成するか、[プロジェクト] > [既存の項目の追加] と移動して既存のコード ファイルをプロジェクトに追加します
//   6. 後ほどこのプロジェクトを再び開く場合、[ファイル] > [開く] > [プロジェクト] と移動して .sln ファイルを選択します
