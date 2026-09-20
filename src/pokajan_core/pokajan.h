#ifndef POKAJAN_H
#define POKAJAN_H

#include "cards.h"
#include <stdbool.h>

#define POKAJAN_MAX_MATCHES 96

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

    Match lastMatch; // empty match if no claims are being made
    bool matchDiscard[5];

    bool discardClaimable;
    bool contestInProgress; // whether a contest is in progress
    Match contestMatch[3]; // the contested matches
    int contestants; // number of contestants

    bool ended;
} Game;

#define EMPTY_MATCH (Match){ -1, THREE_OF_A_KIND, DIFFERENT, { EMPTY_CARD, EMPTY_CARD, EMPTY_CARD, EMPTY_CARD, EMPTY_CARD }, 0, false, -1 }
#define IS_EMPTY_MATCH(a_) ((a_).playerIndex == -1)

void PokajanInit(Game *game);
bool PokajanSetInitialHand(Game *game, int playerIndex, Card hand[7]);

bool PokajanDraw(Game *game, int playerIndex, Card card);
bool PokajanDiscardOnTurn(Game *game, int playerIndex, int from);

int PokajanCheckMatches(Game *game, int playerIndex, Match matches[POKAJAN_MAX_MATCHES]);
bool PokajanCommitSelfMatch(Game *game, int playerIndex, Match match);
bool PokajanDiscardAfterMatch(Game *game, int playerIndex, int from);
bool PokajanReplenish(Game *game, int playerIndex, Card card, int to);
void PokajanEndMatchSequence(Game *game);

bool PokajanDeclareContestOnDiscardMatch(Game *game, int playerIndex, Match match);
bool PokajanResolveContestAndCommitDiscardMatch(Game *game, Match *outWinner);

void PokajanEnd(Game *game);
int PokajanGetWinners(Game *game, int outWinners[4]);

#endif