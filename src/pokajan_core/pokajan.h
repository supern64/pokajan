#ifndef POKAJAN_H
#define POKAJAN_H

#include "cards.h"

typedef struct {
    Card hand[7];
    Card drawnSlot;
    Card lastDiscard;
    int coins;
} Player;

typedef struct {
    Generation generations[4];
    Player players[4];
    int bonusMemID;
    int turnIndex;
    int cards;

    bool matchInProgress;
    int matchClaimant;   // who's currently allowed to draw off-turn
    int drawsRemaining;  // how many replenishment draws are still owed

    bool discardClaimable;
    bool contestInProgress; // whether a contest is in progress
    Match contestMatch[3]; // the contested matches
    int contestants; // number of contestants
} Game;

typedef enum {
    CONTINUE,
    WAIT_DRAW,  // needs player to draw cards
    WAIT_DISCARD, // needs player to discard cards
    WAIT_CONTEST_MATCH // waiting for players to claim or skip pokajan after a discarded card
} ContinueState;

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

// Initializes the game data.
void PokajanInit(Game *game);

// Registers a card as drawn for a player. Closes the claiming period of the previous player's discard if one exists. Must be their turn if Pokajan! has not been claimed. Decreases internal card count. Returns success status.
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

// Registers a match from your own hands, commits the coin transfer, and waits for drawn cards. After player has finished redrawing, check for a Pokajan! again with PokajanCheckMatches(). Returns success status.
bool PokajanCommitSelfMatch(Game *game, Match match);

// Returns whether the game can continue to the next turn, and why if it can't.
ContinueState PokajanGetContinueState(Game *game);

// Returns true if the game will end due to a player having 0 coins or the deck running out of cards.
bool PokajanWillEnd(Game *game);

#endif