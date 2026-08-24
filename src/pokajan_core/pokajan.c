#include "pokajan.h"

// Initializes the game data.
void PokajanInit(Game *game) {
    GetRandomGenerations(game->generations);

    // initialize players
    for (int i = 0; i < 4; i++) {
        game->players[i].coins = 1000;
        for (int j = 0; j < 7; j++) {
            game->players[i].hand[j] = EMPTY_CARD;
        }
        game->players[i].drawnSlot = EMPTY_CARD;
    }

    game->bonusCard = GetRandomBonusCard(game->generations);
    game->turnIndex = 0;
    game->cards = 100;

    game->matchInProgress = false;
    game->matchClaimant = -1;
    game->drawsRemaining = 0;

    game->discardClaimable = false;
    game->contestInProgress = false;
    game->contestants = 0;
}
