#ifndef BRIDGE_H
#define BRIDGE_H

#include <stdbool.h>
#include <stdint.h>
#include "../pokajan_core/cards.h"
#include "../pokajan_core/pokajan.h"

#define MAX_CARDS 567
#define EMPTY_CARD_ID { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }

typedef uint8_t NfcId[7];

typedef enum {
    WAIT_DRAW,
    WAIT_DISCARD,
    WAIT_DECLARE,
    WAIT_TURN
} SeatState;

typedef enum {
    DECLARE,
    SKIP
} DeclareAction;

typedef struct {
    bool online;
    bool handReady;
    SeatState state;
    MemberSlot member;

    NfcId hand[7];
    NfcId drawnSlot;
    NfcId discardSlot;
} Seat;

typedef struct {
    NfcId id;
    Card card;
} CardMap;

typedef CardMap CardTable[MAX_CARDS];

typedef struct {
    Game game;
    Seat seats[4];
    bool allReady; // all 28 initial cards have been placed onto the stands and is ready for the first player's turn to begin.
    struct mosquitto* mosq;
} PokajanTable;

PokajanTable* BridgeGetTable();

int BridgeLoadCardTable(void);
Card BridgeResolveCard(NfcId id);

void BridgeInitTable(PokajanTable *table);

void BridgeOnHandUpdate(PokajanTable *table, int standId, NfcId hand[7]);
void BridgeOnDrawnUpdate(PokajanTable *table, int standId, NfcId drawn);
void BridgeOnDiscardUpdate(PokajanTable *table, int standId, NfcId discard);
void BridgeOnDeclareAction(PokajanTable *table, int standId, DeclareAction action, int target);
void BridgeOnStatusUpdate(PokajanTable *table, int standId, bool online);

#endif