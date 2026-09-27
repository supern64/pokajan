#ifndef NETWORK_H
#define NETWORK_H

#include <stdbool.h>
#include "bridge.h"

#define DEFAULT_IP "127.0.0.1"
#define DEFAULT_PORT 1883

bool NetworkInit(PokajanTable* table);
void NetworkLoop();
void NetworkShutdown();

// Publishes the coin count for a player.
void NetworkPostCoins(PokajanTable *table, int standId, int coins);

// Publishes a player's potential and current match.
void NetworkPostPlayerMatch(PokajanTable *table, int playerIndex, int nMatch, Match matches[POKAJAN_MAX_MATCHES]);

// Publishes the discard LED state.
void NetworkPostDiscardInUse(PokajanTable *table, int playerIndex, DiscardLightState state);

// Publishes the match action state.
void NetworkPostMatchActionRequired(PokajanTable *table, int playerIndex, bool required);

// Publishes the error states for slots.
// (1 << slot) set for slotMask when there is an error, and set for typeMask when the error is an unexpected card instead of a missing card.
void NetworkPostError(PokajanTable *table, int standId, uint16_t slotMask, uint16_t typeMask);

// Publishes the current turn index.
void NetworkPostCurrentTurn(PokajanTable *table, int turnIndex);

// Publishes the current matcher index.
void NetworkPostCurrentMatcher(PokajanTable *table, int matcherIndex);

// Publishes the list of winners.
void NetworkPostWinners(PokajanTable *table, int winnerCount, int winnerIdx[4]);

// Publishes the generations list.
void NetworkPostGenerations(PokajanTable *table, int generations[4]);

// Publishes the deck count.
void NetworkPostDeckCount(PokajanTable *table, int count);

#endif