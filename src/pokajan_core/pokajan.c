#include "pokajan.h"

// Initializes the game data.
void PokajanInit(Game *game) {
    PokajanGetRandomGenerations(game->generations);

    // initialize players
    for (int i = 0; i < 4; i++) {
        game->players[i].coins = 1000;
        for (int j = 0; j < 7; j++) {
            game->players[i].hand[j] = EMPTY_CARD;
        }
        game->players[i].drawnSlot = EMPTY_CARD;
        game->players[i].lastDiscard = EMPTY_CARD;
    }

    game->bonusCard = PokajanGetRandomBonusCard(game->generations);
    game->turnIndex = 0;
    game->cards = 100;

    for (int i = 0; i < 5; i++) game->matchDiscard[i] = false;
    game->lastMatch = EMPTY_MATCH;

    game->discardClaimable = false;
    game->contestInProgress = false;
    for (int i = 0; i < 3; i++) game->contestMatch[i] = EMPTY_MATCH;
    game->contestants = 0;

    game->ended = false;
}

bool PokajanSetInitialHand(Game *game, int playerIndex, Card hand[7]) {
    if (game->ended) return false;
    // invalid if the players do not have all empty cards
    for (int i = 0; i < 7; i++) {
        if (!IS_EMPTY_CARD(game->players[playerIndex].hand[i])) return false;
    }
    for (int i = 0; i < 7; i++) {
        game->players[playerIndex].hand[i] = hand[i];
    }
    game->cards -= 7;
    return true;
}

bool PokajanDraw(Game *game, int playerIndex, Card card) {
    if (game->ended) return false;
    if (game->turnIndex != playerIndex) return false;
    if (!IS_EMPTY_CARD(game->players[playerIndex].drawnSlot)) return false;
    if (game->cards == 0) return false; // detected by bridge
    
    // ensure discard contest is closed
    game->discardClaimable = false;
    game->players[playerIndex].drawnSlot = card;
    game->cards -= 1;
    // bridge waits for discard
    return true;
}

// must wait until the player moves their drawn slot back in and places their card in discard to call
bool PokajanDiscardOnTurn(Game *game, int playerIndex, int from) {
    if (game->ended) return false;
    if (game->turnIndex != playerIndex) return false; // it says it right there. discard ON TURN
    if (from > 7 || from < 0) return false;

    // make sure card exists
    Card target = from == 7 ? game->players[playerIndex].drawnSlot : game->players[playerIndex].hand[from];
    if (IS_EMPTY_CARD(target)) return false;

    // move card from drawn slot into main slot if drawn slot isn't the one discarded
    if (from != 7) {
        game->players[playerIndex].lastDiscard = game->players[playerIndex].hand[from];
        game->players[playerIndex].hand[from] = game->players[playerIndex].drawnSlot;
        game->players[playerIndex].drawnSlot = EMPTY_CARD;
    } else {
        game->players[playerIndex].lastDiscard = game->players[playerIndex].drawnSlot;
        game->players[playerIndex].drawnSlot = EMPTY_CARD;
    }

    game->discardClaimable = true;
    game->turnIndex += 1;
    if (game->turnIndex == 4) game->turnIndex = 0;

    return true;
}

// computes bonus of a match (90 per matched card)
static int PokajanComputeBonus(Game *game, Card *matchCards, int count) {
    int bonus = 0;
    for (int i = 0; i < count; i++) {
        if (IS_EMPTY_CARD(matchCards[i])) continue;
        if (IS_SAME_MEMBER(matchCards[i], game->bonusCard)) {
            bonus += 90;
        }
    }
    return bonus;
}

// Gathers the distinct colors present among `cards` (up to 3) into outColors.
// Duplicates of the same color are collapsed to one entry - interchangeable
// instances don't create meaningfully different declare options.
static int PokajanDistinctColors(Card *cards, int count, int outColors[3]) {
    bool seen[3] = { false, false, false };
    int n = 0;
    for (int i = 0; i < count; i++) {
        int v = cards[i].variant;
        if (v < 0 || v > V_ORANGE) continue;
        if (!seen[v]) { seen[v] = true; outColors[n++] = v; }
    }
    return n;
}
 
// Returns the first card in `cards` matching `color`. Caller must ensure one
// exists (i.e. `color` came from PokajanDistinctColors on this same list).
static Card PokajanFindCardOfColor(Card *cards, int count, int color) {
    for (int i = 0; i < count; i++) {
        if (cards[i].variant == color) return cards[i];
    }
    return cards[0];
}

int PokajanCheckMatches(Game *game, int playerIndex, Match matches[POKAJAN_MAX_MATCHES]) {
    if (playerIndex > 3 || playerIndex < 0) return 0;
    Player *player = &game->players[playerIndex];
    int matchCount = 0;
 
    // "own turn, after drawing" is determined by having an active drawn card,
    // not by turnIndex - the next player's turnIndex advances immediately on
    // discard, before they've drawn, and they still need to react to it.
    bool hasDrawnCard = !IS_EMPTY_CARD(player->drawnSlot);
 
    Card ownCards[8];
    int ownCardsCount = 7;
    for (int i = 0; i < 7; i++) ownCards[i] = player->hand[i];
    if (hasDrawnCard) {
        ownCards[7] = player->drawnSlot;
        ownCardsCount = 8;
    }
 
    // the single live discard, if any, and whether this player may react to it
    bool haveLiveDiscard = false;
    Card liveDiscard = EMPTY_CARD;
    int discarderIndex = (game->turnIndex + 4 - 1) % 4; // whoever just discarded
    if (game->discardClaimable && discarderIndex != playerIndex) {
        liveDiscard = game->players[discarderIndex].lastDiscard;
        if (!IS_EMPTY_CARD(liveDiscard)) haveLiveDiscard = true;
    }
 
    // --- THREE_OF_A_KIND: tally per (generation, member) among active generations ---
    for (int g = 0; g < 4; g++) {
        Generation gen = game->generations[g];
        int memberCount = GENERATION_MEMBER_COUNT[gen];
 
        for (int m = 0; m < memberCount; m++) {
            int memberId = GENERATIONS[gen][m];
            if (memberId == -1) continue;
 
            Card ofMember[9]; // max 3 colors x 3 copies
            int ofMemberCount = 0;
            for (int i = 0; i < ownCardsCount; i++) {
                if (IS_EMPTY_CARD(ownCards[i])) continue;
                if (ownCards[i].generation == gen && ownCards[i].id == memberId) {
                    ofMember[ofMemberCount++] = ownCards[i];
                }
            }
            if (ofMemberCount < 2) continue; // not even a near-match
 
            // group by color for composition enumeration
            Card colorCards[3][3];
            int colorCounts[3] = { 0, 0, 0 };
            for (int i = 0; i < ofMemberCount; i++) {
                int c = ofMember[i].variant;
                if (c >= 0 && c <= V_ORANGE && colorCounts[c] < 3) {
                    colorCards[c][colorCounts[c]++] = ofMember[i];
                }
            }
 
            if (ofMemberCount >= 3) {
                // self-complete: enumerate every achievable 3-card composition
                // (a,b,c) of blue/pink/orange - bounded to at most 10 by
                // stars-and-bars over 3 colors, not by raw card subsets
                for (int a = 0; a <= colorCounts[0] && a <= 3; a++) {
                    for (int b = 0; b <= colorCounts[1] && a + b <= 3; b++) {
                        int c = 3 - a - b;
                        if (c < 0 || c > colorCounts[2]) continue;
                        if (matchCount >= POKAJAN_MAX_MATCHES) continue;
 
                        Card combo[3];
                        int ci = 0;
                        for (int i = 0; i < a; i++) combo[ci++] = colorCards[0][i];
                        for (int i = 0; i < b; i++) combo[ci++] = colorCards[1][i];
                        for (int i = 0; i < c; i++) combo[ci++] = colorCards[2][i];
 
                        Match match = { 0 };
                        match.playerIndex = playerIndex;
                        match.pattern = THREE_OF_A_KIND;
                        match.useDiscardOf = -1;
                        for (int i = 0; i < 5; i++) match.matchInHand[i] = EMPTY_CARD;
                        for (int i = 0; i < 3; i++) match.matchInHand[i] = combo[i];
                        match.complete = true;
                        match.colorState = (a == 3 || b == 3 || c == 3) ? SAME : DIFFERENT;
                        int baseReward = (match.colorState == SAME) ? 840 : 120;
                        match.reward = baseReward + PokajanComputeBonus(game, combo, 3);
                        matches[matchCount++] = match;
                    }
                }
            } else {
                // real near-match (exactly 2 held) - only one possible subset
                if (matchCount < POKAJAN_MAX_MATCHES) {
                    Match match = { 0 };
                    match.playerIndex = playerIndex;
                    match.pattern = THREE_OF_A_KIND;
                    match.useDiscardOf = -1;
                    for (int i = 0; i < 5; i++) match.matchInHand[i] = EMPTY_CARD;
                    for (int i = 0; i < ofMemberCount; i++) match.matchInHand[i] = ofMember[i];
                    match.complete = false;
                    match.colorState = DIFFERENT; // best-effort hint only, not guaranteed
                    match.reward = 0; // undetermined until complete
                    matches[matchCount++] = match;
                }
            }
 
            // discard-boost: enumerate every achievable 2-card own-composition
            // paired with the fixed live discard. Only ever completes an existing
            // near-match (ofMemberCount >= 2, already established) - never
            // manufactures a new one, so this only ever emits complete=true entries.
            if (haveLiveDiscard && liveDiscard.generation == gen && liveDiscard.id == memberId) {
                for (int p = 0; p <= colorCounts[0] && p <= 2; p++) {
                    for (int q = 0; q <= colorCounts[1] && p + q <= 2; q++) {
                        int r = 2 - p - q;
                        if (r < 0 || r > colorCounts[2]) continue;
                        if (matchCount >= POKAJAN_MAX_MATCHES) continue;
 
                        Card ownCombo[2];
                        int ci = 0;
                        for (int i = 0; i < p; i++) ownCombo[ci++] = colorCards[0][i];
                        for (int i = 0; i < q; i++) ownCombo[ci++] = colorCards[1][i];
                        for (int i = 0; i < r; i++) ownCombo[ci++] = colorCards[2][i];
 
                        Card combined[3] = { ownCombo[0], ownCombo[1], liveDiscard };
 
                        Match match = { 0 };
                        match.playerIndex = playerIndex;
                        match.pattern = THREE_OF_A_KIND;
                        match.useDiscardOf = discarderIndex;
                        for (int i = 0; i < 5; i++) match.matchInHand[i] = EMPTY_CARD;
                        match.matchInHand[0] = ownCombo[0];
                        match.matchInHand[1] = ownCombo[1]; // borrowed 3rd lives in useDiscardOf
                        match.complete = true;
                        match.colorState = PokajanIsAllSameColor(combined, 3) ? SAME : DIFFERENT;
                        int baseReward = (match.colorState == SAME) ? 840 : 120;
                        match.reward = baseReward + PokajanComputeBonus(game, combined, 3);
                        matches[matchCount++] = match;
                    }
                }
            }
        }
    }
 
    // --- FULL_GENERATION: presence + full color-assignment enumeration per generation ---
    for (int g = 0; g < 4; g++) {
        Generation gen = game->generations[g];
        int memberCount = GENERATION_MEMBER_COUNT[gen];
 
        Card byMember[5][8];
        int byMemberCount[5] = { 0 };
        bool haveMember[5] = { false };
        int presentCount = 0;
 
        for (int m = 0; m < memberCount; m++) {
            int memberId = GENERATIONS[gen][m];
            if (memberId == -1) continue;
 
            for (int i = 0; i < ownCardsCount; i++) {
                if (IS_EMPTY_CARD(ownCards[i])) continue;
                if (ownCards[i].generation == gen && ownCards[i].id == memberId) {
                    if (byMemberCount[m] < 8) byMember[m][byMemberCount[m]++] = ownCards[i];
                }
            }
            if (byMemberCount[m] > 0) {
                haveMember[m] = true;
                presentCount++;
            }
        }
 
        if (presentCount == memberCount) {
            // self-complete: enumerate every color assignment across members -
            // full cartesian product of each member's distinct held colors, but
            // bounded in practice by the 8-card hand budget, not by colors^members
            int colorOptions[5][3];
            int colorOptionCount[5] = { 0 };
            for (int m = 0; m < memberCount; m++) {
                colorOptionCount[m] = PokajanDistinctColors(byMember[m], byMemberCount[m], colorOptions[m]);
            }
 
            int idx[5] = { 0 };
            bool done = false;
            while (!done) {
                if (matchCount < POKAJAN_MAX_MATCHES) {
                    Card representative[5];
                    bool allSame = true;
                    int firstColor = colorOptions[0][idx[0]];
                    for (int m = 0; m < memberCount; m++) {
                        int chosenColor = colorOptions[m][idx[m]];
                        if (chosenColor != firstColor) allSame = false;
                        representative[m] = PokajanFindCardOfColor(byMember[m], byMemberCount[m], chosenColor);
                    }
 
                    Match match = { 0 };
                    match.playerIndex = playerIndex;
                    match.pattern = FULL_GENERATION;
                    match.useDiscardOf = -1;
                    for (int i = 0; i < 5; i++) match.matchInHand[i] = EMPTY_CARD;
                    for (int i = 0; i < memberCount; i++) match.matchInHand[i] = representative[i];
                    match.complete = true;
                    match.colorState = allSame ? SAME : DIFFERENT;
                    int baseReward;
                    switch (memberCount) {
                        case 3: baseReward = allSame ? 480 : 180; break;
                        case 4: baseReward = allSame ? 840 : 300; break;
                        case 5: baseReward = allSame ? 1800 : 480; break;
                        default: baseReward = 0; break;
                    }
                    match.reward = baseReward + PokajanComputeBonus(game, representative, memberCount);
                    matches[matchCount++] = match;
                }
 
                // odometer increment across all members' color-option indices
                int pos = memberCount - 1;
                while (pos >= 0) {
                    idx[pos]++;
                    if (idx[pos] < colorOptionCount[pos]) break;
                    idx[pos] = 0;
                    pos--;
                }
                if (pos < 0) done = true;
            }
        } else if (presentCount == memberCount - 1) {
            // real near-match (n-1 present) - hint-only representative set
            Card nearRep[5];
            int nearRepCount = 0;
            for (int m = 0; m < memberCount; m++) {
                if (!haveMember[m]) continue;
                nearRep[nearRepCount++] = byMember[m][0];
            }
            if (matchCount < POKAJAN_MAX_MATCHES) {
                Match match = { 0 };
                match.playerIndex = playerIndex;
                match.pattern = FULL_GENERATION;
                match.useDiscardOf = -1;
                for (int i = 0; i < 5; i++) match.matchInHand[i] = EMPTY_CARD;
                for (int i = 0; i < nearRepCount; i++) match.matchInHand[i] = nearRep[i];
                match.complete = false;
                match.colorState = DIFFERENT; // best-effort hint only, not guaranteed
                match.reward = 0;
                matches[matchCount++] = match;
            }
        } else {
            continue; // fewer than n-1 present: no self-complete, near-match, or
                       // discard-boost is reachable regardless of what the discard is
        }
 
        // discard-boost: covers two distinct cases uniformly -
        //  (1) the discard fills the one member missing from an n-1-present hand
        //  (2) the discard offers an alternate color for a member ALREADY present,
        //      letting an already-declarable DIFFERENT set upgrade to SAME (or
        //      offer another DIFFERENT composition) by swapping that one card out
        // Both share the same requirement: every member OTHER than the discard's
        // own member must already be held - so this is one check, not two.
        if (haveLiveDiscard && liveDiscard.generation == gen) {
            int targetSlot = PokajanGetMemberSlot(gen, liveDiscard.id);
            if (targetSlot != -1) {
                bool othersPresent = true;
                for (int m = 0; m < memberCount; m++) {
                    if (m == targetSlot) continue;
                    if (!haveMember[m]) { othersPresent = false; break; }
                }
 
                if (othersPresent) {
                    int otherMembers[5];
                    int numOthers = 0;
                    for (int m = 0; m < memberCount; m++) {
                        if (m == targetSlot) continue;
                        otherMembers[numOthers++] = m;
                    }
 
                    int colorOptions[5][3];
                    int colorOptionCount[5] = { 0 };
                    for (int i = 0; i < numOthers; i++) {
                        int m = otherMembers[i];
                        colorOptionCount[i] = PokajanDistinctColors(byMember[m], byMemberCount[m], colorOptions[i]);
                    }
 
                    int idx[5] = { 0 };
                    bool done = false;
                    while (!done) {
                        if (matchCount < POKAJAN_MAX_MATCHES) {
                            Card representative[5];
                            bool allSameAsDiscard = true;
                            for (int i = 0; i < numOthers; i++) {
                                int m = otherMembers[i];
                                int chosenColor = colorOptions[i][idx[i]];
                                if (chosenColor != liveDiscard.variant) allSameAsDiscard = false;
                                representative[i] = PokajanFindCardOfColor(byMember[m], byMemberCount[m], chosenColor);
                            }
 
                            Card combined[5];
                            for (int i = 0; i < numOthers; i++) combined[i] = representative[i];
                            combined[numOthers] = liveDiscard; // borrowed card only in combined, not matchInHand
 
                            Match match = { 0 };
                            match.playerIndex = playerIndex;
                            match.pattern = FULL_GENERATION;
                            match.useDiscardOf = discarderIndex;
                            for (int i = 0; i < 5; i++) match.matchInHand[i] = EMPTY_CARD;
                            for (int i = 0; i < numOthers; i++) match.matchInHand[i] = representative[i];
                            match.complete = true;
                            match.colorState = allSameAsDiscard ? SAME : DIFFERENT;
                            int baseReward;
                            switch (memberCount) {
                                case 3: baseReward = allSameAsDiscard ? 480 : 180; break;
                                case 4: baseReward = allSameAsDiscard ? 840 : 300; break;
                                case 5: baseReward = allSameAsDiscard ? 1800 : 480; break;
                                default: baseReward = 0; break;
                            }
                            match.reward = baseReward + PokajanComputeBonus(game, combined, numOthers + 1);
                            matches[matchCount++] = match;
                        }
 
                        int pos = numOthers - 1;
                        while (pos >= 0) {
                            idx[pos]++;
                            if (idx[pos] < colorOptionCount[pos]) break;
                            idx[pos] = 0;
                            pos--;
                        }
                        if (pos < 0) done = true;
                    }
                }
            }
        }
    }
 
    return matchCount;
}


static bool PokajanIsHandFull(Card hand[7]) {
    for (int i = 0; i < 7; i++) {
        if (IS_EMPTY_CARD(hand[i])) return false;
    }
    return true;
}

static bool PokajanResolvePayment(Game *game, int from, int to, int amount) {
    if (from > 3 || from < 0 || to > 3 || to < 0) return false;
    int target = game->players[from].coins - amount;
    game->players[to].coins += amount;
    if (target <= 0) {
        game->players[from].coins = 0;
        game->ended = true;
        // immediately ends if someone runs out of coins
    } else {
        game->players[from].coins = target;
    }
    return true;
}

// validates if this match actually exists in player's hand (does not validate reward/complete flag)
static bool PokajanValidateMatch(Game *game, int playerIndex, Match match) {
    Card repr = match.matchInHand[0];
    if (IS_EMPTY_CARD(repr)) return false;
    if (playerIndex > 3 || playerIndex < 0) return false;
    if (match.playerIndex != playerIndex) return false;

    for (int i = 0; i < 5; i++) {
        if (!IS_EMPTY_CARD(match.matchInHand[i]) && match.matchInHand[i].variant > V_ORANGE) return false;
    }

    // repr's generation must actually be one of the 4 active generations -
    // otherwise a match could reference members that aren't even in play
    bool genActive = false;
    for (int i = 0; i < 4; i++) if (game->generations[i] == repr.generation) genActive = true;
    if (!genActive) return false;

    int patternSize = (match.pattern == THREE_OF_A_KIND) ? 3 : GENERATION_MEMBER_COUNT[repr.generation];
    bool usesDiscard = (match.useDiscardOf != -1);
    int expectedOwnSlots = usesDiscard ? patternSize - 1 : patternSize;

    // matchInHand must have exactly expectedOwnSlots populated, contiguously
    // from index 0 - downstream code assumes this shape
    for (int i = 0; i < expectedOwnSlots; i++) if (IS_EMPTY_CARD(match.matchInHand[i])) return false;
    for (int i = expectedOwnSlots; i < 5; i++) if (!IS_EMPTY_CARD(match.matchInHand[i])) return false;

    Card liveDiscard = EMPTY_CARD;
    if (usesDiscard) {
        if (!game->discardClaimable) return false;
        int actualDiscarder = (game->turnIndex + 4 - 1) % 4; // whoever just discarded
        if (actualDiscarder != match.useDiscardOf) return false;
        if (match.useDiscardOf == playerIndex) return false; // can't borrow your own discard
        liveDiscard = game->players[match.useDiscardOf].lastDiscard;
        if (IS_EMPTY_CARD(liveDiscard)) return false;
    }

    if (match.pattern == THREE_OF_A_KIND) {
        int matchColors[3] = { 0, 0, 0 }; // includes the borrowed discard, if any
        for (int i = 0; i < expectedOwnSlots; i++) {
            if (!IS_SAME_MEMBER(repr, match.matchInHand[i])) return false;
            matchColors[match.matchInHand[i].variant]++;
        }
        if (usesDiscard) {
            if (!IS_SAME_MEMBER(repr, liveDiscard)) return false;
            matchColors[liveDiscard.variant]++;
        }

        int handColors[3] = { 0, 0, 0 };
        for (int i = 0; i < 7; i++) {
            Card *c = &game->players[playerIndex].hand[i];
            if (IS_SAME_MEMBER(*c, repr)) handColors[c->variant]++;
        }
        Card *drawn = &game->players[playerIndex].drawnSlot;
        if (!IS_EMPTY_CARD(*drawn) && IS_SAME_MEMBER(*drawn, repr)) handColors[drawn->variant]++;

        // the hand only needs to cover the OWN-contributed portion - the
        // borrowed discard card was never "in hand" to begin with
        int ownColors[3] = { matchColors[0], matchColors[1], matchColors[2] };
        if (usesDiscard) ownColors[liveDiscard.variant]--;
        for (int i = 0; i < 3; i++) {
            if (ownColors[i] > handColors[i]) return false;
        }

        if (match.colorState == SAME) {
            int nonZero = 0;
            for (int i = 0; i < 3; i++) if (matchColors[i] > 0) nonZero++;
            if (nonZero != 1) return false; // all 3 contributing cards share one color
        }
        return true;
    } else {
        int genCount = GENERATION_MEMBER_COUNT[repr.generation];
        int matchColors[5][3] = { 0 }; // [member][color], includes borrowed discard
        int handColors[5][3] = { 0 };

        for (int i = 0; i < expectedOwnSlots; i++) {
            Card *c = &match.matchInHand[i];
            if (c->generation != repr.generation) return false;
            int slot = PokajanGetMemberSlot(c->generation, c->id);
            if (slot < 0 || slot >= genCount) return false;
            matchColors[slot][c->variant]++;
        }

        int discardSlot = -1;
        if (usesDiscard) {
            if (liveDiscard.generation != repr.generation) return false;
            discardSlot = PokajanGetMemberSlot(liveDiscard.generation, liveDiscard.id);
            if (discardSlot < 0 || discardSlot >= genCount) return false;
            matchColors[discardSlot][liveDiscard.variant]++;
        }

        for (int i = 0; i < 7; i++) {
            Card *c = &game->players[playerIndex].hand[i];
            if (c->generation != repr.generation) continue;
            int slot = PokajanGetMemberSlot(c->generation, c->id);
            if (slot < 0 || slot >= genCount) continue;
            handColors[slot][c->variant]++;
        }
        Card *drawn = &game->players[playerIndex].drawnSlot;
        if (!IS_EMPTY_CARD((*drawn)) && drawn->generation == repr.generation) {
            int slot = PokajanGetMemberSlot(drawn->generation, drawn->id);
            if (slot >= 0 && slot < genCount) handColors[slot][drawn->variant]++;
        }

        // every member of the generation must be represented exactly once, and
        // that representation must be backed by hand - except the one member
        // coming from the borrowed discard, which isn't "in hand"
        for (int m = 0; m < genCount; m++) {
            int totalForMember = 0;
            for (int c = 0; c < 3; c++) totalForMember += matchColors[m][c];
            if (totalForMember != 1) return false;

            for (int c = 0; c < 3; c++) {
                if (matchColors[m][c] == 0) continue;
                int needed = matchColors[m][c];
                if (m == discardSlot && c == liveDiscard.variant) needed -= 1;
                if (needed > handColors[m][c]) return false;
            }
        }

        if (match.colorState == SAME) {
            int color = -1;
            for (int m = 0; m < genCount; m++) {
                for (int c = 0; c < 3; c++) {
                    if (matchColors[m][c] > 0) {
                        if (color == -1) color = c;
                        else if (color != c) return false;
                    }
                }
            }
        }
        return true;
    }
}


bool PokajanCommitSelfMatch(Game *game, int playerIndex, Match match) {
    if (game->ended) return false;
    // someone already claiming, so reject everyone else
    if (!IS_EMPTY_MATCH(game->lastMatch) && game->lastMatch.playerIndex != playerIndex) return false;

    if (IS_EMPTY_MATCH(game->lastMatch)) {
        // only triggers on self draw, since contest won will already have match claimant set
        if (game->turnIndex != playerIndex) return false;
    } else {
        // all cards from last match must be discarded first
        int i = 0;
        while (i < 5 && !IS_EMPTY_CARD(game->lastMatch.matchInHand[i])) {
            if (!game->matchDiscard[i]) return false;
            i++;
        }
    }

    // player must replenish all cards in hand before calling another pokajan (enforced by bridge)
    if (!PokajanIsHandFull(game->players[playerIndex].hand)) return false;

    if (!PokajanValidateMatch(game, playerIndex, match)) return false;

    int fromEachPlayer = match.reward / 3;
    for (int i = 0; i < 4; i++) {
        if (i == playerIndex) continue;
        PokajanResolvePayment(game, i, playerIndex, fromEachPlayer);
    }

    // wait for bridge to pick out the cards consumed
    game->lastMatch = match;
    for (int i = 0; i < 5; i++) game->matchDiscard[i] = false;
    return true;
}

bool PokajanDiscardAfterMatch(Game *game, int playerIndex, int from) {
    if (game->ended) return false;
    if (game->lastMatch.playerIndex != playerIndex) return false;
    if (from > 7 || from < 0) return false;

    Card *target = (from == 7) ? &game->players[playerIndex].drawnSlot : &game->players[playerIndex].hand[from];
    if (IS_EMPTY_CARD(*target)) return false;

    int i = 0;
    bool found = false;
    while (i < 5 && !IS_EMPTY_CARD(game->lastMatch.matchInHand[i])) {
        if (IS_SAME_CARD(*target, game->lastMatch.matchInHand[i]) && !game->matchDiscard[i]) {
            found = true;
            game->matchDiscard[i] = true;
            break;
        }
        i++;
    }

    if (!found) return false;

    if (from == 7) {
        game->players[playerIndex].drawnSlot = EMPTY_CARD;
    } else {
        game->players[playerIndex].hand[from] = EMPTY_CARD;
    }

    return true;
}

// CANNOT REPLENISH INTO DRAWN SLOT
bool PokajanReplenish(Game *game, int playerIndex, Card card, int to) {
    if (game->ended) return false;
    if (game->lastMatch.playerIndex != playerIndex) return false;
    if (game->cards == 0) return false; // game ends instantly (detected by bridge)
    if (to > 6 || to < 0) return false;

    if (!IS_EMPTY_CARD(game->players[playerIndex].hand[to])) return false;
    // requires you discard all cards from your last match first
    int i = 0;
    while (i < 5 && !IS_EMPTY_CARD(game->lastMatch.matchInHand[i])) {
        if (!game->matchDiscard[i]) return false;
        i++;
    }

    // only replenishes up to max hand
    if (PokajanIsHandFull(game->players[playerIndex].hand)) return false;

    game->players[playerIndex].hand[to] = card;
    game->cards--;
    return true;
}

void PokajanEndMatchSequence(Game *game) {
    if (game->ended) return;
    game->lastMatch = EMPTY_MATCH;
    for (int i = 0; i < 5; i++) game->matchDiscard[i] = false;
}

bool PokajanDeclareContestOnDiscardMatch(Game *game, int playerIndex, Match match) {
    int lastDiscarderIdx = ((game->turnIndex + 4 - 1) % 4);
    if (game->ended || !game->discardClaimable || !IS_EMPTY_MATCH(game->lastMatch) || match.playerIndex == lastDiscarderIdx || match.useDiscardOf != lastDiscarderIdx) return false;
    if (game->contestants == 3) return false;

    for (int i = 0; i < game->contestants; i++) {
        if (game->contestMatch[i].playerIndex == match.playerIndex) return false;
    }

    if (!PokajanValidateMatch(game, playerIndex, match)) return false;
    game->contestInProgress = true;
    game->contestMatch[game->contestants++] = match;
    return true;
}

bool PokajanResolveContestAndCommitDiscardMatch(Game *game, Match *outWinner) {
    if (game->ended || !game->contestInProgress || game->contestants == 0) return false;
    int lastDiscardIdx = (game->turnIndex + 4 - 1) % 4;

    // will never trigger if nobody contests...
    if (game->contestants == 1) {
        // only one contestant, instantly wins
        *outWinner = game->contestMatch[0];
    } else {
        int order[game->contestants];
        for (int i = 0; i < game->contestants; i++) order[i] = i;

        // bias the reward by distance from target player to instantly resolve ties (+3 max, so shouldn't affect actual scoring)
        int biasReward[game->contestants]; 
        for (int i = 0; i < game->contestants; i++) {
            // CW distance
            biasReward[i] = game->contestMatch[i].reward - ((game->contestMatch[i].playerIndex + 4 - lastDiscardIdx) % 4);
        }

        for (int i = 1; i < game->contestants; i++) {
            int key = order[i];
            int keyReward = biasReward[i];
            int j = i - 1;
            while (j >= 0 && biasReward[order[j]] < keyReward) {
                order[j + 1] = order[j];
                j--;
            }
            order[j + 1] = key;
        }

        *outWinner = game->contestMatch[order[0]];
    }
    
    PokajanResolvePayment(game, lastDiscardIdx, outWinner->playerIndex, outWinner->reward);

    game->contestInProgress = false;
    for (int i = 0; i < 3; i++) game->contestMatch[i] = EMPTY_MATCH;
    game->contestants = 0;

    // setup chain
    game->lastMatch = *outWinner;
    for (int i = 0; i < 5; i++) game->matchDiscard[i] = false;
    return true;
}

void PokajanEnd(Game *game) {
    game->ended = true;
}

int PokajanGetWinners(Game *game, int outWinners[4]) {
    if (!game->ended) return 0;
    int order[4] = {0, 1, 2, 3};

    // Sort player indices by coins descending (simple insertion sort, only 4 elements)
    for (int i = 1; i < 4; i++) {
        int key = order[i];
        int keyCoins = game->players[key].coins;
        int j = i - 1;
        while (j >= 0 && game->players[order[j]].coins < keyCoins) {
            order[j + 1] = order[j];
            j--;
        }
        order[j + 1] = key;
    }

    // Assign ranks, handling ties (equal coins => equal rank)
    int winnerCount = 1;
    outWinners[0] = order[0];
    for (int i = 1; i < 4; i++) {
        if (game->players[order[i]].coins == game->players[order[i - 1]].coins) {
            outWinners[i] = order[i];
            winnerCount++;
        } else {
            break;
        }
    }
    return winnerCount;
}

int PokajanGetGenerationSlot(Game *game, Generation generation) {
    for (int i = 0; i < 4; i++) {
        if (game->generations[i] == generation) return i;
    }
    return -1;
}