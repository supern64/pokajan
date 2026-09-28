#ifndef BRIDGE_H
#define BRIDGE_H

#include <stdbool.h>
#include <stdint.h>
#include "../pokajan_core/cards.h"
#include "../pokajan_core/pokajan.h"

typedef uint8_t NfcId[7];

#define MAX_CARDS 567
#define EMPTY_CARD_ID ((const NfcId){ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF })
#define IS_SAME_CARD_ID(a_, b_) (memcmp((a_), (b_), sizeof(NfcId)) == 0)
#define SET_CARD_ID(addr_, card_) (memcpy((addr_), (card_), sizeof(NfcId)))
#define IS_CARD_ID_EMPTY(id_) IS_SAME_CARD_ID((id_), EMPTY_CARD_ID)

// declarations
typedef enum {
    DECLARE,
    SKIP
} DeclareAction;

// card table
typedef struct {
    NfcId id;
    Card card;
} CardMap;

typedef CardMap CardTable[MAX_CARDS];

// expect
#define MAX_EXPECT 32
#define SLOT(s_) ((uint16_t)(1u << (s_))) // bitmask to indicate slots
#define HAND_SLOTS ((uint16_t)0x007F)
#define FULL_SLOTS ((uint16_t)0x00FF)

#define SLOT_DRAWN 7
#define SLOT_DISCARD 8

typedef enum {
    EXPECT_REMOVE,
    EXPECT_PUT,
    EXPECT_MOVE
} ExpectAction;

typedef struct {
    ExpectAction action;

    int srcStand;
    uint16_t srcMask;
    int dstStand;
    uint16_t dstMask;
    int dstFromExpectSrc;

    Card filter;

    // filled in by solver to use later on
    bool done;
    bool vacuous;
    int srcSlot;
    int dstSlot;
    NfcId uid;
    Card resolvedCard;
} Expect;

// events
#define MAX_EVENTS 64

typedef enum {
    EVENT_GAME_START,       // all initial hands placed
    EVENT_TURN_START,       // standId = whose turn
    EVENT_DRAW,             // standId, card
    EVENT_DISCARD,          // standId, card
    EVENT_CONTEST_OPEN,     // standId = discarder, eligibleMask
    EVENT_CONTEST_CLOSED,   // standId = discarder, nobody declared
    EVENT_POKAJAN,          // standId, match, fromDiscardOf, coinsBefore/After
    EVENT_MATCH_END,        // standId = matcher
    EVENT_ACTION_REJECTED,  // standId tried to act with a mismatched stand
    EVENT_GAME_END          // winners, winnerCount, coinsAfter = final coins
} TableEventType;

typedef struct {
    TableEventType type;
    int standId;
    int fromDiscardOf;      // -1 unless a Pokajan! used someone's discard
    uint8_t eligibleMask;   // (1 << standId) per eligible contestant
    Card card;
    Match match;
    int coinsBefore[4];
    int coinsAfter[4];
    int winners[4];
    int winnerCount;
} TableEvent;

// various states
typedef enum {
    WAIT_INITIAL_HANDS,
    WAIT_DRAW,
    WAIT_SELF_DECISION,
    WAIT_DISCARD,
    WAIT_CONTEST,
    MATCH_REMOVE,
    MATCH_REPLENISH,
    WAIT_CHAIN_DECISION,
    ENDED
} TableState;

typedef enum {
    SLOT_OK,
    SLOT_MISSING,
    SLOT_UNEXPECTED
} SlotState;

typedef enum {
    NOT_IN_PLAY,
    CONTESTABLE,
    USED_IN_CONTEST
} DiscardLightState;

typedef struct {
    bool online;
    MemberSlot member;

    NfcId observed[9];
    Card observedCard[9];
    NfcId shadow[9];
    Card shadowCard[9];
    bool shuffled;

    SlotState slotState[9];
    bool mismatch;

    Match matches[POKAJAN_MAX_MATCHES];
    int matchCount;

    // contests
    bool contestEligible;
    bool contestDecided;

    bool actionRequired;

    uint16_t lastErrorSlots;
    uint16_t lastErrorTypes;

    DiscardLightState discardLight;
} Seat;

typedef struct {
    Game game;
    Seat seats[4];
    struct mosquitto* mosq;

    TableState state;

    // tracking game states
    unsigned transitions;

    Expect expect[MAX_EXPECT];
    int expectCount;

    bool anyMismatch;

    int discarderId;

    // chaining
    int matcherId;
    bool chainFromOwnTurn;
    uint16_t vacatedSlots;
    bool shortDeck; // deck ran out of cards for replenish, game ends

    // events (see BridgePollEvent)
    TableEvent events[MAX_EVENTS];
    int eventHead;
    int eventCount;
} PokajanTable;

PokajanTable* BridgeGetTable();

int BridgeLoadCardTable(void);
Card BridgeResolveCard(NfcId id);

void BridgeInitTable(PokajanTable *table);

void BridgeRepublishSeat(PokajanTable *table, int s);
void BridgeRepublishState(PokajanTable *table);

void BridgeOnSlotUpdate(PokajanTable *table, int standId, int from, int count, NfcId* cards);
void BridgeOnDeclareAction(PokajanTable *table, int standId, DeclareAction action, int target);
void BridgeOnStatusUpdate(PokajanTable *table, int standId, bool online);

bool BridgePollEvent(PokajanTable *table, TableEvent *outEvent);

#endif