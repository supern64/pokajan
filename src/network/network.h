#ifndef NETWORK_H
#define NETWORK_H

#include <stdbool.h>
#include "bridge.h"

#define DEFAULT_IP "127.0.0.1"
#define DEFAULT_PORT 1883

typedef enum {
    NOT_IN_PLAY,
    CONTESTABLE,
    USED_IN_CONTEST
} DiscardLightState;

bool NetworkInit(PokajanTable* table);
void NetworkLoop();
void NetworkShutdown();

// Publishes coins, current turn, generations and deck count.
void NetworkPostGameState(PokajanTable *table);

// Publishes a player's potential and current match.
void NetworkPostPlayerMatch(PokajanTable *table, int playerIndex, int nMatch, Match matches[POKAJAN_MAX_MATCHES]);

// Publishes the discard LED state.
void NetworkPostDiscardInUse(PokajanTable *table, int playerIndex, DiscardLightState state);

// Publishes the match action state.
void NetworkPostMatchActionRequired(PokajanTable *table, int playerIndex, bool required);

#endif