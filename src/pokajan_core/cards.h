#ifndef CARDS_H
#define CARDS_H

#include <stdbool.h>
#include "member_data.h"

typedef enum {
	V_BLUE,
	V_PINK,
	V_ORANGE,

	// only used for the board, does not exist as a real card
	V_UNCOLORED,
	V_DISPLAY
} Variant;

typedef struct {
	int id;						// -1 indicates an empty card
	Generation generation;
	Variant variant;
} Card;


#define EMPTY_CARD (Card){ .id = -1, .generation = -1, .variant = V_UNCOLORED }
#define IS_EMPTY_CARD(card_) ((card_).id == -1)
#define IS_SAME_CARD(a_, b_) (((a_).id == (b_).id && (a_).generation == (b_).generation && (a_).variant == (b_).variant) || ((a_).id == -1 && (b_).id == -1))
#define IS_SAME_MEMBER(a_, b_) ((a_).id == (b_).id && (a_).generation == (b_).generation)

// Gets 4 random valid generations for a Pokajan! game.
void PokajanGetRandomGenerations(Generation generations[4]);

// Gets a random bonus card from a set of generations.
Card PokajanGetRandomBonusCard(Generation generations[4]);

// Checks if a set of cards all have the same member and color. Empty cards ignored.
bool PokajanIsAllSameCard(Card *cards, int count);

#endif

