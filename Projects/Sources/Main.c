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
    eGamePhaseTest,             // テストフェーズ
} eGamePhase;
#if defined(_DEBUG)
#   define FIRST_PHASE  eGamePhasePlayer    //eGamePhaseTest
#else
#   define FIRST_PHASE  eGamePhasePlayer
#endif


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
static eGamePhase phaseTest();


//
// Works
//
static const PhaseInfo PHASE_INFO[] = {     // フェイズ情報
    {eGamePhasePlayer,  phasePlayer},
    {eGamePhaseResult,  phaseResult},
    {eGamePhaseTest,    phaseTest},
    {eGamePhaseEnd, NULL},  // 終端判定
};
static eGamePhase m_gamePhase;              // ゲームフェイズ

static DeckData m_deck;                     // デッキデータ
static PlayData m_playerData;               // プレイヤーデータ
static int m_changeHandCount;               // 手札の交換回数


// プレイデータ用のカードの役を計算
BOOL calcPokerHandForPlayData(PlayData* const playDataP)
{
    if (!playDataP)
    {
        return FALSE;
    }

    return calcPokerHand(playDataP->cards, playDataP->cardCount, &playDataP->hand);
}

// 初期化
BOOL initialize()
{
    int i;
    const Card initCard = {
        .type = eCardTypeNil,
        .number = 0,
        .priority = 0,
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
    CHK(calcPokerHandForPlayData(&m_playerData));

    // 交換可能回数を設定
    m_changeHandCount = MAX_CHANGE_HAND_CNT;

    // 初期フェイズを設定
    m_gamePhase = FIRST_PHASE;

    return TRUE;
}

// カートタイプ名を取得
const char* const getCardTypeName(eCardType type)
{
    typedef struct {
        eCardType type;
        const char* const name;
    } CardTypeInfo;
    const CardTypeInfo cardTypeInfoList[] = {
        {.type = eCardTypeSpade,        .name = "S"},
        {.type = eCardTypeClover,       .name = "C"},
        {.type = eCardTypeHeart,        .name = "H"},
        {.type = eCardTypeDiamond,      .name = "D"},
        {.type = eCardTypeNil,          .name = "unknow"},
    };
    const CardTypeInfo* typeP = &cardTypeInfoList[0];

    while (typeP->type != eCardTypeNil)
    {
        if (typeP->type == type)
        {
            break;
        }
        typeP++;
    }

    return typeP->name;
}

// ポーカーの役名を取得
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
void printPlayData(const PlayData* const playDataP, const char* const playerNameP)
{
    int i;

    {
        PRINT("%s ", playerNameP);
        PRINT("[hand:%s]", getPokerHandName(playDataP->hand));
        for (i = 0; i < playDataP->cardCount; ++i)
        {
            PRINT("[No.%d: %s%d]", i, getCardTypeName(playDataP->cards[i].type), playDataP->cards[i].number);
        }
        PRINT("\n");
    }
    PRINT("\n");
}

// プレイヤーフェイズ
static eGamePhase phasePlayer()
{
    int i;
    char* changeSelect;
    char changeSelectBuf[8] = "\0";
    eGamePhase phase = eGamePhasePlayer;

    LOG("--- phase player\n");
    printPlayData(&m_playerData, "■プレイヤー");

    if (0 < m_changeHandCount)
    {
        PRINT("> %d回まで交換可能(%d回目)\n", MAX_CHANGE_HAND_CNT, (MAX_CHANGE_HAND_CNT - m_changeHandCount + 1));
        PRINT("> 交換しない場合は %s を入力してください\n", "N");
        PRINT("> 交換するカード番号を選んでください (例：023) : ");
        scanf_s("%s", changeSelectBuf, (unsigned int)sizeof(changeSelectBuf));
        if (changeSelectBuf[0] == 'N' || changeSelectBuf[0] == 'n')
        {
            m_changeHandCount = 0;
        }
        else
        {
            // カード交換
            changeSelect = changeSelectBuf;
            while (*changeSelect != '\0')
            {
                i = changeSelect[0] - '0';
                if (0 <= i && i < PLAYER_MAX_CARD_CNT)
                {
                    CHK(drawDeck(&m_deck, &m_playerData.cards[i]));
                }
                else
                {
                    PRINT("入力された値が不正です [%s]\n", changeSelectBuf);
                    break;
                }
                changeSelect++;
            }
            CHK(calcPokerHandForPlayData(&m_playerData));

            m_changeHandCount--;
        }
    }
    else
    {
        phase = eGamePhaseResult;
    }

    return phase;
}

// リザルトフェイズ
static eGamePhase phaseResult()
{
    LOG("--- phase result\n");
    printPlayData(&m_playerData, "■結果発表");
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

// テストフェイズ
static eGamePhase phaseTest()
{
    PlayData testData;
    testData.cardCount = 5;

    LOG("--- phase test\n");
    {
        testData.cards[0].type = eCardTypeSpade;
        testData.cards[0].number = 1;
        testData.cards[0].priority = 14;
        testData.cards[1].type = eCardTypeSpade;
        testData.cards[1].number = 2;
        testData.cards[1].priority = 2;
        testData.cards[2].type = eCardTypeSpade;
        testData.cards[2].number = 3;
        testData.cards[2].priority = 3;
        testData.cards[3].type = eCardTypeSpade;
        testData.cards[3].number = 4;
        testData.cards[3].priority = 4;
        testData.cards[4].type = eCardTypeHeart;
        testData.cards[4].number = 1;
        testData.cards[4].priority = 14;
        CHK(calcPokerHandForPlayData(&testData));
        printPlayData(&testData, "■ワンペア");
    }
    {
        testData.cards[0].type = eCardTypeSpade;
        testData.cards[0].number = 1;
        testData.cards[0].priority = 14;
        testData.cards[1].type = eCardTypeSpade;
        testData.cards[1].number = 2;
        testData.cards[1].priority = 2;
        testData.cards[2].type = eCardTypeSpade;
        testData.cards[2].number = 3;
        testData.cards[2].priority = 3;
        testData.cards[3].type = eCardTypeHeart;
        testData.cards[3].number = 2;
        testData.cards[3].priority = 2;
        testData.cards[4].type = eCardTypeHeart;
        testData.cards[4].number = 1;
        testData.cards[4].priority = 14;
        CHK(calcPokerHandForPlayData(&testData));
        printPlayData(&testData, "■ツーペア");
    }
    {
        testData.cards[0].type = eCardTypeSpade;
        testData.cards[0].number = 1;
        testData.cards[0].priority = 14;
        testData.cards[1].type = eCardTypeSpade;
        testData.cards[1].number = 2;
        testData.cards[1].priority = 2;
        testData.cards[2].type = eCardTypeSpade;
        testData.cards[2].number = 3;
        testData.cards[2].priority = 3;
        testData.cards[3].type = eCardTypeClover;
        testData.cards[3].number = 1;
        testData.cards[3].priority = 14;
        testData.cards[4].type = eCardTypeHeart;
        testData.cards[4].number = 1;
        testData.cards[4].priority = 14;
        CHK(calcPokerHandForPlayData(&testData));
        printPlayData(&testData, "■スリーカード");
    }
    {
        testData.cards[0].type = eCardTypeSpade;
        testData.cards[0].number = 1;
        testData.cards[0].priority = 14;
        testData.cards[1].type = eCardTypeSpade;
        testData.cards[1].number = 2;
        testData.cards[1].priority = 2;
        testData.cards[2].type = eCardTypeDiamond;
        testData.cards[2].number = 1;
        testData.cards[2].priority = 14;
        testData.cards[3].type = eCardTypeClover;
        testData.cards[3].number = 1;
        testData.cards[3].priority = 14;
        testData.cards[4].type = eCardTypeHeart;
        testData.cards[4].number = 1;
        testData.cards[4].priority = 14;
        CHK(calcPokerHandForPlayData(&testData));
        printPlayData(&testData, "■フォーカード");
    }
    {
        testData.cards[0].type = eCardTypeSpade;
        testData.cards[0].number = 6;
        testData.cards[0].priority = 6;
        testData.cards[1].type = eCardTypeSpade;
        testData.cards[1].number = 2;
        testData.cards[1].priority = 2;
        testData.cards[2].type = eCardTypeDiamond;
        testData.cards[2].number = 3;
        testData.cards[2].priority = 3;
        testData.cards[3].type = eCardTypeClover;
        testData.cards[3].number = 4;
        testData.cards[3].priority = 4;
        testData.cards[4].type = eCardTypeHeart;
        testData.cards[4].number = 5;
        testData.cards[4].priority = 5;
        CHK(calcPokerHandForPlayData(&testData));
        printPlayData(&testData, "■ストレート");
    }
    {
        testData.cards[0].type = eCardTypeSpade;
        testData.cards[0].number = 1;
        testData.cards[0].priority = 14;
        testData.cards[1].type = eCardTypeSpade;
        testData.cards[1].number = 2;
        testData.cards[1].priority = 2;
        testData.cards[2].type = eCardTypeSpade;
        testData.cards[2].number = 3;
        testData.cards[2].priority = 3;
        testData.cards[3].type = eCardTypeSpade;
        testData.cards[3].number = 4;
        testData.cards[3].priority = 4;
        testData.cards[4].type = eCardTypeSpade;
        testData.cards[4].number = 5;
        testData.cards[4].priority = 5;
        CHK(calcPokerHandForPlayData(&testData));
        printPlayData(&testData, "■フラッシュ");
    }
    {
        testData.cards[0].type = eCardTypeSpade;
        testData.cards[0].number = 1;
        testData.cards[0].priority = 14;
        testData.cards[1].type = eCardTypeHeart;
        testData.cards[1].number = 1;
        testData.cards[1].priority = 14;
        testData.cards[2].type = eCardTypeClover;
        testData.cards[2].number = 1;
        testData.cards[2].priority = 14;
        testData.cards[3].type = eCardTypeSpade;
        testData.cards[3].number = 4;
        testData.cards[3].priority = 4;
        testData.cards[4].type = eCardTypeHeart;
        testData.cards[4].number = 4;
        testData.cards[4].priority = 4;
        CHK(calcPokerHandForPlayData(&testData));
        printPlayData(&testData, "■フルハウス");
    }
    {
        testData.cards[0].type = eCardTypeSpade;
        testData.cards[0].number = 2;
        testData.cards[0].priority = 2;
        testData.cards[1].type = eCardTypeSpade;
        testData.cards[1].number = 3;
        testData.cards[1].priority = 3;
        testData.cards[2].type = eCardTypeSpade;
        testData.cards[2].number = 4;
        testData.cards[2].priority = 4;
        testData.cards[3].type = eCardTypeSpade;
        testData.cards[3].number = 5;
        testData.cards[3].priority = 5;
        testData.cards[4].type = eCardTypeSpade;
        testData.cards[4].number = 6;
        testData.cards[4].priority = 6;
        CHK(calcPokerHandForPlayData(&testData));
        printPlayData(&testData, "■ストレートフラッシュ");
    }
    {
        testData.cards[0].type = eCardTypeSpade;
        testData.cards[0].number = 10;
        testData.cards[0].priority = 10;
        testData.cards[1].type = eCardTypeSpade;
        testData.cards[1].number = 11;
        testData.cards[1].priority = 11;
        testData.cards[2].type = eCardTypeSpade;
        testData.cards[2].number = 12;
        testData.cards[2].priority = 12;
        testData.cards[3].type = eCardTypeSpade;
        testData.cards[3].number = 13;
        testData.cards[3].priority = 13;
        testData.cards[4].type = eCardTypeSpade;
        testData.cards[4].number = 1;
        testData.cards[4].priority = 14;
        CHK(calcPokerHandForPlayData(&testData));
        printPlayData(&testData, "■ロイヤルストレートフラッシュ");
    }
    {
        testData.cards[0].type = eCardTypeSpade;
        testData.cards[0].number = 1;
        testData.cards[0].priority = 14;
        testData.cards[1].type = eCardTypeSpade;
        testData.cards[1].number = 2;
        testData.cards[1].priority = 2;
        testData.cards[2].type = eCardTypeDiamond;
        testData.cards[2].number = 3;
        testData.cards[2].priority = 3;
        testData.cards[3].type = eCardTypeClover;
        testData.cards[3].number = 4;
        testData.cards[3].priority = 4;
        testData.cards[4].type = eCardTypeHeart;
        testData.cards[4].number = 5;
        testData.cards[4].priority = 5;
        CHK(calcPokerHandForPlayData(&testData));
        printPlayData(&testData, "■ストレートもどき");
    }

    PRINT("\n\n");

    return eGamePhaseEnd;
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
