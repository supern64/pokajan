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
        game->players[i].lastDiscard = EMPTY_CARD;
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

    game->ended = false;
    game->endPending = false;
}

bool PokajanSetInitialHand(Game *game, int playerIndex, Card hand[7]) {
    if (game->ended || game->endPending) return false;
    // invalid if the players do not have all empty cards
    for (int i = 0; i < 7; i++) {
        if (!IS_CARD_EMPTY(game->players[playerIndex].hand[i])) return false;
    }
    for (int i = 0; i < 7; i++) {
        game->players[playerIndex].hand[i] = hand[i];
    }
    game->cards -= 7;
    return true;
}

bool PokajanDraw(Game *game, int playerIndex, Card card) {
    if (game->ended || game->endPending) return false;
    if (game->turnIndex != playerIndex) return false;
    if (!IS_CARD_EMPTY(game->players[playerIndex].drawnSlot)) return false;
    // ensure discard contest is closed
    game->discardClaimable = false;
    game->players[playerIndex].drawnSlot = card;
    game->cards -= 1;
    // bridge waits for discard
    return true;
}

bool PokajanDiscardOnTurn(Game *game, int playerIndex, Card card) {
    if (game->ended) return false;
    if (game->turnIndex != playerIndex) return false; // it says it right there. discard ON TURN
    int cardIndex = -1;
    // make sure card exists
    for (int i = 0; i < 7; i++) {
        if (IS_SAME_CARD(game->players[playerIndex].hand[i], card)) {
            cardIndex = i;
            break;
        }
    }
    // drawn slot
    if (cardIndex == -1 && IS_SAME_CARD(game->players[playerIndex].drawnSlot, card)) cardIndex = 7;

    if (cardIndex == -1) return false; // cannot discard, it doesn't exist

    // move card from drawn slot into main slot if drawn slot isn't the one discarded
    if (cardIndex != 7) {
        game->players[playerIndex].lastDiscard = game->players[playerIndex].hand[playerIndex];
        game->players[playerIndex].hand[playerIndex] = game->players[playerIndex].drawnSlot;
        game->players[playerIndex].drawnSlot = EMPTY_CARD;
    } else {
        game->players[playerIndex].lastDiscard = game->players[playerIndex].drawnSlot;
        game->players[playerIndex].drawnSlot = EMPTY_CARD;
    }

    // check ending flag set by PokajanReplenish/PokajanCommitSelfMatch/PokajanResolveContestAndCommitDiscardMatch
    if (game->endPending || PokajanWillEnd(game)) {
        game->ended = true;
        // determine winners, game's over
    } else {
        game->discardClaimable = true;
        game->turnIndex += 1;
        if (game->turnIndex == 4) game->turnIndex = 0;
    }
}

bool PokajanWillEnd(Game *game) {
    if (game->cards == 0) return true;
    // one player has 0 coins
    for (int i = 0; i < 4; i++) {
        if (game->players[i].coins == 0) return true;
    }
    return false;
}