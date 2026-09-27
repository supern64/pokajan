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

typedef enum {
    DECLARE,
    SKIP
} DeclareAction;

typedef struct {
    NfcId id;
    Card card;
} CardMap;

typedef CardMap CardTable[MAX_CARDS];

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

#endif