#include "cards.h"
#include <raylib.h>
#include <stdio.h>

void PokajanGetRandomGenerations(Generation generations[4]) {
    while (1) {
        int selectedGen[15] = { 0 };
        int totalMembers = 0;
        int count = 0;

        while (count < 4) {
            int toPick = GetRandomValue(0, 14);
            if (!selectedGen[toPick]) {
                selectedGen[toPick] = 1;
                generations[count++] = toPick;
                totalMembers += GENERATION_MEMBER_COUNT[toPick];
            }
        }

        // Rule 1: GEN_1 and GAMERS cannot be together
        if (selectedGen[GEN_1] && selectedGen[GAMERS]) continue;

        // Rule 2: Total members must be between 15 and 17
        if (totalMembers >= 15 && totalMembers <= 17) {
            break; // Valid combination
        }
    }
}

Card PokajanGetRandomBonusCard(Generation generations[4]) {
    int pickGen = generations[GetRandomValue(0, 3)];
    int pickMem = GENERATIONS[pickGen][GetRandomValue(0, GENERATION_MEMBER_COUNT[pickGen] - 1)];

    return (Card) {
        .id = pickMem,
        .generation = pickGen,
        .variant = V_UNCOLORED
    };
}

bool PokajanIsAllSameColor(Card *cards, int count) {
    Card compare = EMPTY_CARD;
    for (int i = 0; i < count; i++) {
        if (IS_EMPTY_CARD(cards[i])) continue;
        if (IS_EMPTY_CARD(compare)) {
            compare = cards[i];
            continue;
        }

        if (!IS_SAME_CARD(compare, cards[i])) {
            return false;
        }
    }
    return true;
}

int PokajanGetMemberSlot(Generation generation, int id) {
    for (int i = 0; i < GENERATION_MEMBER_COUNT[generation]; i++) {
        if (GENERATIONS[generation][i] == id) return i;
    }
    return -1;
}