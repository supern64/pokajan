#ifndef POKAJAN_H
#define POKAJAN_H

#include "cards.h"
#include <stdbool.h>

typedef enum {
    THREE_OF_A_KIND,
    FULL_GENERATION
} MatchPattern;

typedef enum {
    SAME, // every single color is the same
    DIFFERENT // any color is different
} MatchColor;

typedef struct {
    int playerIndex; // equal to -1 if match does not exist
    MatchPattern pattern;
    MatchColor colorState;
    Card matchInHand[5]; // only counts cards in hand
    int reward;
    bool complete; // whether this match is available to claim
    int useDiscardOf; // player index of discard, -1 if all self
} Match;

typedef struct {
    Card hand[7];
    Card drawnSlot;
    Card lastDiscard;
    int coins;
} Player;

typedef struct {
    Generation generations[4];
    Player players[4];
    Card bonusCard;
    int turnIndex;
    int cards;

    bool matchInProgress;
    int matchClaimant; // invalid (-1) if matchInProgress is false
    int drawsRemaining;

    bool discardClaimable;
    bool contestInProgress; // whether a contest is in progress
    Match contestMatch[3]; // the contested matches
    int contestants; // number of contestants

    bool endPending;
    bool ended;
} Game;

void PokajanInit(Game *game);
bool PokajanSetInitialHand(Game *game, int playerIndex, Card hand[7]);

bool PokajanDraw(Game *game, int playerIndex, Card card);
bool PokajanDiscardOnTurn(Game *game, int playerIndex, Card card);

int PokajanCheckMatches(Game *game, int playerIndex, Match matches[20]);
bool PokajanCommitSelfMatch(Game *game, Match match);
bool PokajanDiscardAfterMatch(Game *game, int playerIndex, Card card);
bool PokajanReplenish(Game *game, int playerIndex, Card card, int slot);
void PokajanEndMatchSequence(Game *game);

bool PokajanDeclareContestOnDiscardMatch(Game *game, int playerIndex, Match match);
bool PokajanResolveContestAndCommitDiscardMatch(Game *game, Match *outWinner);

bool PokajanWillEnd(Game *game);
bool PokajanIsEnded(Game *game);
void PokajanEnd(Game *game);
int PokajanGetWinners(Game *game, int outWinners[4]);

#endif