#include "cards.h"
#include <raylib.h>
#include <stdio.h>

// Gets 4 random valid generations for a Pokajan! game.
void GetRandomGenerations(Generation generations[4]) {
    int selectedGen[15] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    int currentMemberCount = 0;

    int i = 0;
    while (i < 4) {
        int toPick = GetRandomValue(0, 14);

        // must be unique
        if (selectedGen[toPick]) continue;

        // JP1 and GAMERS cannot be together
        if ((toPick == GEN_1 && selectedGen[GAMERS]) || (toPick == GAMERS && selectedGen[GEN_1])) continue;

        int membersInGen = GENERATION_MEMBER_COUNT[toPick];
        if (currentMemberCount > 10 && membersInGen == 5) continue;
        if (currentMemberCount < 8 && membersInGen == 3) continue;
        
        currentMemberCount += membersInGen;
        selectedGen[toPick] = 1;
        generations[i] = toPick;
        i++;
    }
}

// Gets a random bonus card from a set of generations.
Card GetRandomBonusCard(Generation generations[4]) {
    int pickGen = generations[GetRandomValue(0, 3)];
    int pickMem = GENERATIONS[pickGen][GetRandomValue(0, GENERATION_MEMBER_COUNT[pickGen] - 1)];

    return (Card) {
        .id = pickMem,
        .generation = pickGen,
        .variant = V_UNCOLORED
    };
}