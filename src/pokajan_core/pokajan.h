#ifndef POKAJAN_H
#define POKAJAN_H

#include "cards.h"
#include <stdbool.h>

typedef enum {
    THREE_OF_A_KIND, FULL_GENERATION
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
    int useDiscardOf; // if the match involves a discard, this will be set to the player index whose discard was used.
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
} Game;

/*
    (Game = main app loop, Engine = this)
    Engine Loop:
        - Initialize engine with PokajanInit, initialize generations and bonus and shows them to the players
        - After the generations are shown, instructions are shown to create a 100-card deck (and this number is reflected in engine state)
        - 7 cards are physically dealt out to each player.
          Each card placed in a slot will call PokajanInsertDraw, and the engine should wait with WAIT_DRAW until all 28 draws are complete.
        - Turn operation goes as follows:
            - Player physically draws a card from the deck into the drawn slot, calls PokajanDraw
            - Game checks eligible matches for the single player with PokajanCheckMatches
                - If the player is eligible, the game will display options to Pokajan!
                - If the player decides to Pokajan!, the game calls


*/
// Initializes the game data.
void PokajanInit(Game *game);

bool PokajanSetInitialHand(Game *game, int playerIndex, Card hand[7]);

// Marks a card as drawn and puts it into the specified slot. Does not include turn draws into the hand slot.
bool PokajanInsertDraw(Game *game, int playerIndex, Card card, int slot);

// Registers a card draw into the drawn slot for a player. Closes the claiming period of the previous player's discard if one exists. Must be their turn if Pokajan! has not been claimed. Decreases internal card count. Returns success status.
bool PokajanDraw(Game *game, int playerIndex, Card card);

// Registers a discarded card for a player's turn. Must be their turn. Run when the player puts the card in the discard slot. Returns success status.
bool PokajanDiscardOnTurn(Game *game, int playerIndex, Card card);

// Registers discarded cards from a claimed match. Can only run if Pokajan! has been claimed. Runs whenever the player removes a card used for a match. Returns success status.
bool PokajanDiscardAfterMatch(Game *game, int playerIndex, Card card);

// Checks potential and possible matches for a player. Returns the number of matches.
int PokajanCheckMatches(Game *game, int playerIndex, Match matches[20]);

// Declares a contest on a discarded card. May only be done after a turn discard. Returns success status.
bool PokajanDeclareContestOnDiscardMatch(Game *game, int playerIndex, Match match);

/* 
    Resolves the contest and commits the coin transfer.
    Clears the contestant/contest in progress fields and waits for the winner to draw cards.
    After winner has finished redrawing, check for a Pokajan! again with PokajanCheckMatches().
    Returns false if no coin transfer happened due to no winner/nonexistent contest.
*/
bool PokajanResolveContestAndCommitDiscardMatch(Game *game, Match *outWinner);

// Called after a match chain when no more matches are possible.
// Clears matchInProgress/matchClaimant/drawsRemaining.
void PokajanEndMatchSequence(Game *game);

// Registers a match from your own hands, commits the coin transfer, and waits for drawn cards. After player has finished redrawing, check for a Pokajan! again with PokajanCheckMatches(). Returns success status.
bool PokajanCommitSelfMatch(Game *game, Match match);

// Returns true if the game will end due to a player having 0 coins or the deck running out of cards.
bool PokajanWillEnd(Game *game);

#endif