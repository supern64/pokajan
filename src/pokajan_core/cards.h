#ifndef CARDS_H
#define CARDS_H

typedef enum {
	GEN_0, GEN_1, GEN_2, GAMERS, GEN_3, GEN_4, GEN_5, HOLOX,
	MYTH, PROMISE, ADVENT,
	ID_GEN_1, ID_GEN_2, ID_GEN_3,
	REGLOSS
} Generation;

typedef enum {
	V_BLUE,
	V_PINK,
	V_ORANGE,

	// only used for the board, does not exist as a real card
	V_UNCOLORED,
	V_DISPLAY
} Variant;

typedef int GenerationEntry[5];

typedef struct {
	int id;						// -1 indicates an empty card
	Generation generation;
	Variant variant;
} Card;

#define EMPTY_CARD (Card){ .id = -1, .generation = -1, .variant = V_UNCOLORED }
#define IS_CARD_EMPTY(card_) card_.id == -1

// -1 indicates an empty slot.
static const GenerationEntry GENERATIONS[15] = {
	{  1,  2, 13, 15, 18 }, // JP0
	{  4,  5,  6,  7, -1 }, // JP1
	{  9, 10, 11, 12, -1 }, // JP2
	{  6, 14, 16, 17, -1 }, // GAMERS
	{ 19, 21, 22, 23, -1 }, // JP3
	{ 24, 26, 27, 28, -1 }, // JP4
	{ 30, 31, 32, 34, -1 }, // JP5
	{ 35, 36, 37, 38, 39 }, // HoloX

	{ 4001, 4002, 4003, 4004, 4005 }, // Myth
	{ 4007, 4008, 4009, 4010, 4012 }, // Promise
	{ 4013, 4014, 4015, 4016, 4017 }, // Advent

	{ 3001, 3002, 3003,   -1,   -1 }, // ID1
	{ 3004, 3005, 3006,   -1,   -1 }, // ID2
	{ 3007, 3008, 3009,   -1,   -1 }, // ID3

	{ 6001, 6002, 6003, 6004, 6005 }  // ReGLOSS
};

static const int GENERATION_MEMBER_COUNT[15] = { 5, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 3, 3, 3, 5 };

// Gets 4 random valid generations for a Pokajan! game.
void GetRandomGenerations(Generation generations[4]);

// Gets a random bonus card from a set of generations.
Card GetRandomBonusCard(Generation generations[4]);

#endif

