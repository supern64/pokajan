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

bool PokajanIsAllSameCard(Card *cards, int count) {
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
    if (id == -1 || generation < 0 || generation > REGLOSS) return -1;
    for (int i = 0; i < GENERATION_MEMBER_COUNT[generation]; i++) {
        if (GENERATIONS[generation][i] == id) return i;
    }
    return -1;
}

static LinearOrder LINEAR_ORDERS[2]; // [playableOnly]
static bool linearOrdersBuilt = false;

static void PokajanBuildOne(LinearOrder *out, bool playableOnly) {
    out->count = 0;
    for (int gen = 0; gen < GENERATION_COUNT; gen++) {
        for (int slot = 0; slot < 5; slot++) out->orderOf[gen][slot] = -1;
    }

    for (int gen = 0; gen < GENERATION_COUNT; gen++) {
        for (int slot = 0; slot < GENERATION_MEMBER_COUNT[gen]; slot++) {
            int member = GENERATIONS[gen][slot];
            if (member == -1) continue;
            if (gen == GAMERS && member == 6) continue;
            if (playableOnly && !PLAYABLE_MEMBERS[gen][slot]) continue;

            int order = out->count++;
            out->ids[order] = member;
            out->gens[order] = gen;
            out->slots[order] = slot;
            out->orderOf[gen][slot] = order;
        }
    }
}

void PokajanBuildLinearOrders(void) {
    if (linearOrdersBuilt) return;
    PokajanBuildOne(&LINEAR_ORDERS[0], false);
    PokajanBuildOne(&LINEAR_ORDERS[1], true);
    linearOrdersBuilt = true;
}

MemberSlot PokajanLinearOrderToMember(int order, bool playableOnly) {
    const LinearOrder *lo = &LINEAR_ORDERS[playableOnly ? 1 : 0];
    if (order < 0 || order >= lo->count) return EMPTY_MEMBER;
    return (MemberSlot){
        .order = order,
        .id = lo->ids[order],
        .generation = lo->gens[order],
        .slot = lo->slots[order]
    };
}

int PokajanMemberIdToLinearOrder(int id, bool playableOnly) {
    const LinearOrder *lo = &LINEAR_ORDERS[playableOnly ? 1 : 0];
    for (int i = 0; i < lo->count; i++) {
        if (lo->ids[i] == id) return i;
    }
    return -1;
}

int PokajanGetPlayableCount(void) {
    return LINEAR_ORDERS[1].count;
}