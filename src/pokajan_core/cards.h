#ifndef CARDS_H
#define CARDS_H

#include <stdbool.h>

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
#define IS_EMPTY_CARD(card_) ((card_).id == -1)
#define IS_SAME_CARD(a_, b_) ((a_).id == (b_).id && (a_).generation == (b_).generation && (a_).variant == (b_).variant)
#define IS_SAME_MEMBER(a_, b_) ((a_).id == (b_).id && (a_).generation == (b_).generation)

#define GENERATION_COUNT 15
#define MEMBER_COUNT 63

typedef struct {
    int count;
    int ids[MEMBER_COUNT];            // order -> member id
    int gens[MEMBER_COUNT];           // order -> generation
    int slots[MEMBER_COUNT];          // order -> slot within that generation
    int orderOf[GENERATION_COUNT][5]; // [gen][slot] -> order, -1 if excluded
} LinearOrder;

typedef struct {
	int order;
	int id;
	Generation generation;
	int slot;
} MemberSlot;

#define EMPTY_MEMBER (MemberSlot){ .order = -1, .id = -1, .generation = -1, .slot = -1 };
#define IS_EMPTY_MEMBER(mem_) ((mem_).order == -1)

// -1 indicates an empty slot.
static const GenerationEntry GENERATIONS[GENERATION_COUNT] = {
	{  1,  2, 13, 15, 18 }, // JP0
	{  4,  5,  6,  7, -1 }, // JP1
	{  9, 10, 11, 12, -1 }, // JP2
	{  6, 14, 16, 17, -1 }, // GAMERS
	{ 19, 21, 22, 23, -1 }, // JP3
	{ 24, 26, 27, 28, -1 }, // JP4
	{ 30, 31, 32, 34, -1 }, // JP5
	{ 35, 36, 37, 38, 39 }, // HoloX

	{ 4001, 4002, 4003, 4004, 4005 }, // Myth
	{ 4007, 4009, 4010, 4011, 4012 }, // Promise
	{ 4013, 4014, 4015, 4016, 4017 }, // Advent

	{ 3001, 3002, 3003,   -1,   -1 }, // ID1
	{ 3004, 3005, 3006,   -1,   -1 }, // ID2
	{ 3007, 3008, 3009,   -1,   -1 }, // ID3

	{ 6001, 6002, 6003, 6004, 6005 }  // ReGLOSS
};

// some mems appear in pokajan but do not appear in voices
static const bool PLAYABLE_MEMBERS[GENERATION_COUNT][5] = {
	{  true,  true,  true,  true,  true }, // JP0
	{  true,  true,  true,  true, false }, // JP1
	{ false,  true,  true,  true, false }, // JP2
	{  true,  true,  true,  true, false }, // GAMERS
	{  true,  true,  true,  true, false }, // JP3
	{ false,  true,  true,  true, false }, // JP4
	{  true,  true,  true,  true, false }, // JP5
	{  true,  true,  true, false,  true }, // HoloX

	{  true,  true,  true, false, false }, // Myth
	{  true, false, false,  true,  true }, // Promise
	{  true,  true,  true,  true,  true }, // Advent

	{  true,  true,  true, false, false }, // ID1
	{  true,  true,  true, false, false }, // ID2
	{  true,  true,  true, false, false }, // ID3

	{ false,  true,  true,  true,  true }  // ReGLOSS
};

static const int GENERATION_MEMBER_COUNT[GENERATION_COUNT] = { 5, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 3, 3, 3, 5 };

// Gets 4 random valid generations for a Pokajan! game.
void PokajanGetRandomGenerations(Generation generations[4]);

// Gets a random bonus card from a set of generations.
Card PokajanGetRandomBonusCard(Generation generations[4]);

// Checks if a set of cards all have the same member and color. Empty cards ignored.
bool PokajanIsAllSameCard(Card *cards, int count);

// Gets the index of a member of a generation from their ID. Returns -1 if member is not part of generation.
int PokajanGetMemberSlot(Generation generation, int id);

// Builds the linear order entry for lookup. Should be called once on startup.
void PokajanBuildLinearOrders(void);

// Gets a member slot based on linear order.
MemberSlot PokajanLinearOrderToMember(int order, bool playableOnly);

// Gets the linear order based on the member's ID.
int PokajanMemberIdToLinearOrder(int id, bool playableOnly);

// Gets the number of members marked as playable. (No. of all members defined in MEMBER_COUNT)
int PokajanGetPlayableCount(void);

#endif

