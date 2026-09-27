#include "bridge.h"
#include "network.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <mosquitto.h>
#include <raylib.h>

static CardTable CTable;
static int loadedCount = 0;
PokajanTable Table;

PokajanTable* BridgeGetTable() {
    return &Table;
}

// NfcId to engine card lookup

int BridgeLoadCardTable() {
    FILE *f = fopen("card_table.csv", "r");
    if (f == NULL) return -1;
    char line[32];
    int i = 0;
    while (fgets(line, sizeof(line), f) && i < MAX_CARDS) {
        char uid_hex[15];
        int id;
        uint8_t gen;
        uint8_t var;
        bool reject = false;

        if (sscanf(line, "%14[^,],%d,%hhu,%hhu", uid_hex, &id, &gen, &var) != 4) {
            continue;
        }

        if (strlen(uid_hex) != 14) continue;
        for (int j = 0; j < 14; j++) {
            if (!isxdigit((unsigned char)uid_hex[j])) {
                reject = true;
                break;
            }
        }
        if (reject) continue;

        for (int j = 0; j < 7; j++) {
            if (sscanf(uid_hex + j*2, "%2hhx", &CTable[i].id[j]) != 1) {
                reject = true;
                break;
            }
        }
        if (reject) continue;

        CTable[i].card.id = id;
        CTable[i].card.generation = gen;
        CTable[i].card.variant = var;
        i++;
    }
    loadedCount = i;
    fclose(f);
    return i;
}

Card BridgeResolveCard(NfcId id) {
    if (IS_CARD_ID_EMPTY(id)) return EMPTY_CARD;
    for (int i = 0; i < loadedCount; i++) {
        if (IS_SAME_CARD_ID(CTable[i].id, id)) {
            return CTable[i].card;
        }
    }
    return EMPTY_CARD;
}

// utilities

static bool BridgeFilterOk(Card filter, Card c) {
    return IS_EMPTY_CARD(filter) || IS_SAME_CARD(filter, c);
}

static bool BridgeNfcIdInShadow(const PokajanTable *table, const uint8_t *id) {
    for (int s = 0; s < 4; s++) {
        for (int i = 0; i < 9; i++) {
            if (IS_SAME_CARD_ID(table->seats[s].shadow[i], id)) return true;
        }
    }
    return false;
}

static bool BridgeNfcIdInObserved(const PokajanTable *table, const uint8_t *id) {
    for (int s = 0; s < 4; s++) {
        for (int i = 0; i < 9; i++) {
            if (IS_SAME_CARD_ID(table->seats[s].observed[i], id)) return true;
        }
    }
    return false;
}


static void BridgePostCoins(PokajanTable *table) {
    for (int s = 0; s < 4; s++) {
        NetworkPostCoins(table, s, table->game.players[s].coins);
    }
}

static void BridgePostSeatSlotStatus(PokajanTable *table, int s, bool force) {
    uint16_t slotMask = 0u, typeMask = 0u;
    for (int i = 0; i < 9; i++) {
        if (table->seats[s].slotState[i] != SLOT_OK) {
            slotMask |= SLOT(i);
            if (table->seats[s].slotState[i] == SLOT_UNEXPECTED) typeMask |= SLOT(i);
        }
    }
    if (force || slotMask != table->seats[s].lastErrorSlots || typeMask != table->seats[s].lastErrorTypes) {
        table->seats[s].lastErrorSlots = slotMask;
        table->seats[s].lastErrorTypes = typeMask;
        NetworkPostError(table, s, slotMask, typeMask);
    }
}

static void BridgePostSlotStatus(PokajanTable *table, bool force) {
    for (int s = 0; s < 4; s++) BridgePostSeatSlotStatus(table, s, force);
}

static void BridgePostSeatActionRequired(PokajanTable *table, int s, bool force) {
    bool required = false;
    switch (table->state) {
        case WAIT_SELF_DECISION:
        case WAIT_CHAIN_DECISION:
            required = (s == table->matcherId);
            break;
        case WAIT_CONTEST:
            required = table->seats[s].contestEligible && !table->seats[s].contestDecided;
            break;
        default:
            break;
    }
    if (force || required != table->seats[s].actionRequired) {
        table->seats[s].actionRequired = required;
        NetworkPostMatchActionRequired(table, s, required);
    }
}

static void BridgeUpdateActionRequired(PokajanTable *table, bool force) {
    for (int s = 0; s < 4; s++) BridgePostSeatActionRequired(table, s, force);
}

static void BridgeSetDiscardLight(PokajanTable *table, int standId, DiscardLightState st) {
    table->seats[standId].discardLight = st;
    NetworkPostDiscardInUse(table, standId, st);
}

static void BridgeRefreshMatches(PokajanTable *table, int standId) {
    table->seats[standId].matchCount = PokajanCheckMatches(&table->game, standId, table->seats[standId].matches);
    NetworkPostPlayerMatch(table, standId, table->seats[standId].matchCount, table->seats[standId].matches);
}

static bool BridgeHasSelfComplete(const PokajanTable *table, int standId) {
    for (int i = 0; i < table->seats[standId].matchCount; i++) {
        if (table->seats[standId].matches[i].complete && table->seats[standId].matches[i].useDiscardOf == -1) return true;
    }
    return false;
}

static bool BridgeHasDiscardComplete(const PokajanTable *table, int standId, int discarder) {
    for (int i = 0; i < table->seats[standId].matchCount; i++) {
        if (table->seats[standId].matches[i].complete && table->seats[standId].matches[i].useDiscardOf == discarder) return true;
    }
    return false;
}

static int BridgeCurrentMatcher(const PokajanTable *table) {
    switch (table->state) {
        case MATCH_REMOVE:
        case MATCH_REPLENISH:
        case WAIT_CHAIN_DECISION:
            return table->matcherId;
        default:
            return -1;
    }
}

// manage your expectations (hah)

static Expect* BridgeNewExpect(PokajanTable *table, ExpectAction action) {
    if (table->expectCount >= MAX_EXPECT) return NULL;
    Expect *e = &table->expect[table->expectCount++];
    memset(e, 0, sizeof *e);
    e->action = action;
    e->srcStand = e->dstStand = -1;
    e->dstFromExpectSrc = -1;
    e->filter = EMPTY_CARD;
    e->srcSlot = e->dstSlot = -1;
    return e;
}

static void BridgeExpectRemove(PokajanTable *table, int standId, uint16_t srcMask, Card filter) {
    Expect *e = BridgeNewExpect(table, EXPECT_REMOVE);
    if (!e) return;
    e->srcStand = standId;
    e->srcMask = srcMask;
    e->filter = filter;
}

static void BridgeExpectPut(PokajanTable *table, int standId, uint16_t dstMask, Card filter) {
    Expect *e = BridgeNewExpect(table, EXPECT_PUT);
    if (!e) return;
    e->dstStand = standId;
    e->dstMask = dstMask;
    e->filter = filter;
}

// returns the index of the expect for future reference
static int BridgeExpectMove(PokajanTable *table, int srcStand, uint16_t srcMask, int dstStand, uint16_t dstMask, int dstFromExpectSrc) {
    if (dstFromExpectSrc >= 0) {
        if (dstFromExpectSrc >= table->expectCount) return -1; // must exist
        if (table->expect[dstFromExpectSrc].action == EXPECT_PUT) return -1; // must be an entry that vacates a slot
    }

    Expect *e = BridgeNewExpect(table, EXPECT_MOVE);
    if (!e) return -1;
    e->srcStand = srcStand;
    e->srcMask = srcMask;
    e->dstStand = dstStand;
    e->dstMask = dstMask;
    e->dstFromExpectSrc = dstFromExpectSrc;
    return (int)(e - table->expect);
}

// diffs the shadow and observed card, and resolves the expect group
static void BridgeSolveDiff(PokajanTable *table) {
    bool srcUsed[4][9] = { 0 };
    bool dstUsed[4][9] = { 0 };

    // reset slots
    for (int i = 0; i < table->expectCount; i++) {
        Expect *e = &table->expect[i];
        e->done = e->vacuous = false;
        e->srcSlot = e->dstSlot = -1;
        SET_CARD_ID(e->uid, EMPTY_CARD_ID);
        e->resolvedCard = EMPTY_CARD;
    }

    for (int i = 0; i < table->expectCount; i++) {
        Expect *e = &table->expect[i];

        if (e->action == EXPECT_MOVE) {
            uint16_t dstMask = e->dstMask;

            if (e->dstFromExpectSrc >= 0) {
                Expect *dep = &table->expect[e->dstFromExpectSrc];
                if (!dep->done) continue; // do not evaluate if dependency isn't resolved. will not happen if all entries are valid.
                if (dep->srcStand == e->srcStand && (e->srcMask & SLOT(dep->srcSlot))) {
                    e->done = e->vacuous = true; // dependency already took our source
                    continue;
                }
                dstMask = SLOT(dep->srcSlot);
            }

            // card IDs are unique, so if some card ID is seen in a different slot it is assumed to be moved
            for (int d = 0; d < 9 && !e->done; d++) {
                if (!(dstMask & SLOT(d))) continue;
                const uint8_t *u = table->seats[e->dstStand].observed[d];

                if (IS_CARD_ID_EMPTY(u) || dstUsed[e->dstStand][d]) continue;
                if (IS_SAME_CARD_ID(table->seats[e->dstStand].shadow[d], u)) continue; // nothing arrived here

                for (int s = 0; s < 9; s++) {
                    if (!(e->srcMask & SLOT(s))) continue;
                    if (srcUsed[e->srcStand][s]) continue;
                    if (!IS_SAME_CARD_ID(table->seats[e->srcStand].shadow[s], u)) continue;
                    if (!BridgeFilterOk(e->filter, table->seats[e->srcStand].shadowCard[s])) continue;

                    e->done = true;
                    e->srcSlot = s;
                    e->dstSlot = d;
                    SET_CARD_ID(e->uid, u);
                    e->resolvedCard = table->seats[e->srcStand].shadowCard[s];

                    srcUsed[e->srcStand][s] = true;
                    dstUsed[e->dstStand][d] = true;
                    break;
                }
            }
        } else if (e->action == EXPECT_PUT) {
            for (int d = 0; d < 9; d++) {
                if (!(e->dstMask & SLOT(d))) continue;
                const uint8_t *u = table->seats[e->dstStand].observed[d];

                if (IS_CARD_ID_EMPTY(u) || dstUsed[e->dstStand][d]) continue;
                if (IS_SAME_CARD_ID(table->seats[e->dstStand].shadow[d], u)) continue;
                if (BridgeNfcIdInShadow(table, u)) continue; // old card

                Card c = table->seats[e->dstStand].observedCard[d];
                if (IS_EMPTY_CARD(c)) continue; // unknown card
                if (!BridgeFilterOk(e->filter, c)) continue;

                e->done = true;
                e->dstSlot = d;
                SET_CARD_ID(e->uid, u);
                e->resolvedCard = c;
                dstUsed[e->dstStand][d] = true;
                break;
            }
        } else { // EXPECT_REMOVE
            for (int s = 0; s < 9; s++) {
                if (!(e->srcMask & SLOT(s))) continue;
                const uint8_t *u = table->seats[e->srcStand].shadow[s];

                if (IS_CARD_ID_EMPTY(u) || srcUsed[e->srcStand][s]) continue;
                if (BridgeNfcIdInObserved(table, u)) continue; // must have left the board
                if (!BridgeFilterOk(e->filter, table->seats[e->srcStand].shadowCard[s])) continue;

                e->done = true;
                e->srcSlot = s;
                SET_CARD_ID(e->uid, u);
                e->resolvedCard = table->seats[e->srcStand].shadowCard[s];
                srcUsed[e->srcStand][s] = true;
                break;
            }
        }
    }

    // resolve any removal of discard slots automatically
    for (int s = 0; s < 4; s++) {
        const uint8_t *u = table->seats[s].shadow[SLOT_DISCARD];
        if (IS_CARD_ID_EMPTY(u) || srcUsed[s][SLOT_DISCARD]) continue;
        if (table->state == WAIT_CONTEST && s == table->discarderId) continue;
        if (BridgeNfcIdInObserved(table, u)) continue; // moved somewhere else: not a removal
        srcUsed[s][SLOT_DISCARD] = true;
    }

    table->anyMismatch = false;
    for (int s = 0; s < 4; s++) {
        table->seats[s].mismatch = false;
        for (int i = 0; i < 9; i++) {
            SlotState st = SLOT_OK;
            if (!IS_SAME_CARD_ID(table->seats[s].observed[i], table->seats[s].shadow[i])) {
                if (!IS_CARD_ID_EMPTY(table->seats[s].observed[i]) && !dstUsed[s][i]){
                    st = SLOT_UNEXPECTED;
                } else if (!IS_CARD_ID_EMPTY(table->seats[s].shadow[i]) && !srcUsed[s][i]) {
                    st = SLOT_MISSING;
                }
            }
            table->seats[s].slotState[i] = st;
            if (st != SLOT_OK) {
                table->seats[s].mismatch = true;
                table->anyMismatch = true;
            }
        }
    }
}

static bool BridgeExpectGroupComplete(const PokajanTable *table) {
    if (table->expectCount == 0 || table->anyMismatch) return false;
    for (int i = 0; i < table->expectCount; i++) if (!table->expect[i].done) return false;
    return true;
}

// states and transitions

static void BridgeTransitionTo(PokajanTable *table, TableState next);

// arms the expectation for some state
static void BridgeEnterState(PokajanTable *table) {
    Game *g = &table->game;

    switch (table->state) {
        case WAIT_INITIAL_HANDS:
            for (int s = 0; s < 4; s++) {
                for (int i = 0; i < 7; i++) {
                    BridgeExpectPut(table, s, HAND_SLOTS, EMPTY_CARD);
                }
            }
            break;

        case WAIT_DRAW:
            if (g->ended || g->cards == 0) { BridgeTransitionTo(table, ENDED); return; }
            NetworkPostCurrentTurn(table, g->turnIndex);
            NetworkPostCurrentMatcher(table, -1);
            BridgeExpectPut(table, g->turnIndex, SLOT(SLOT_DRAWN), EMPTY_CARD); // any card into drawn slot
            break;

        case WAIT_SELF_DECISION:
            table->matcherId = g->turnIndex; // right to decide to match
            break;
        
        case WAIT_DISCARD: { // on turn
            int p = g->turnIndex;

            int pick = BridgeExpectMove(table, p, FULL_SLOTS, p, SLOT(SLOT_DISCARD), -1); // expect a move from full slots into discard...
            BridgeExpectMove(table, p, SLOT(SLOT_DRAWN), p, 0, pick); // ...then from drawn into full slots. (will autoresolve if removed card was drawn card)
            break;
        }

        case WAIT_CONTEST:
            break;

        case MATCH_REMOVE: {
            Match *m = &g->lastMatch;

            for (int i = 0; i < 5; i++) {
                if (IS_EMPTY_CARD(m->matchInHand[i])) break;
                BridgeExpectRemove(table, m->playerIndex, FULL_SLOTS, m->matchInHand[i]);
            }
            break;
        }
        
        case MATCH_REPLENISH: {
            int p = table->matcherId;
            uint16_t handHoles = table->vacatedSlots & HAND_SLOTS;
            int needCount = 0;
            for (int i = 0; i < 7; i++) if (handHoles & SLOT(i)) needCount++;
            bool needDrawn = (table->vacatedSlots & SLOT(SLOT_DRAWN)) && table->chainFromOwnTurn;

            int available = g->cards;
            int placeUpTo = needCount < available ? needCount : available;
            for (int i = 0; i < placeUpTo; i++) BridgeExpectPut(table, p, handHoles, EMPTY_CARD);
            if (needDrawn && available > placeUpTo) BridgeExpectPut(table, p, SLOT(SLOT_DRAWN), EMPTY_CARD);

            table->shortDeck = (placeUpTo < needCount) || (needDrawn && available <= needCount);
            if (table->expectCount == 0) { BridgeTransitionTo(table, ENDED); return; }
            break;
        }

        case ENDED:
            PokajanEnd(g);
            NetworkPostCurrentTurn(table, -1);
            NetworkPostCurrentMatcher(table, -1);

            int winners[4];
            int winnerCount = PokajanGetWinners(g, winners);
            NetworkPostWinners(table, winnerCount, winners);
            break;

        default:
            break;
    }
}

static void BridgeTransitionTo(PokajanTable *table, TableState next) {
    table->state = next;
    table->transitions++;
    table->expectCount = 0;
    BridgeEnterState(table); // may transition again (e.g. straight to ST_ENDED)
    BridgeSolveDiff(table);      // cards moved early may already satisfy the new group
    BridgePostSlotStatus(table, true);
    BridgeUpdateActionRequired(table, false);
}

static void BridgeFinishChain(PokajanTable *table) {
    PokajanEndMatchSequence(&table->game);
    NetworkPostCurrentMatcher(table, -1);
    BridgeTransitionTo(table, table->chainFromOwnTurn ? WAIT_DISCARD : WAIT_DRAW);
}

static void BridgeCompleteState(PokajanTable *table) {
    Game *g = &table->game;

    switch (table->state) {
        case WAIT_INITIAL_HANDS:
            for (int s = 0; s < 4; s++) {
                Card hand[7];
                for (int i = 0; i < 7; i++) hand[i] = table->seats[s].shadowCard[i];
                PokajanSetInitialHand(g, s, hand);
            }
            NetworkPostDeckCount(table, g->cards);
            BridgeTransitionTo(table, WAIT_DRAW);
            break;

        case WAIT_DRAW: {
            int p = g->turnIndex;
            PokajanDraw(g, p, table->expect[0].resolvedCard);
            NetworkPostDeckCount(table, g->cards);
            BridgeRefreshMatches(table, p);
            BridgeTransitionTo(table, BridgeHasSelfComplete(table, p) ? WAIT_SELF_DECISION : WAIT_DISCARD);
            break;
        }

        case WAIT_DISCARD: {
            int p = g->turnIndex;
            PokajanDiscardOnTurn(g, p, table->expect[0].srcSlot);
            table->discarderId = p;

            bool anyEligible = false;
            for (int q = 0; q < 4; q++) {
                BridgeRefreshMatches(table, q); // discard reaction
                table->seats[q].contestDecided = false;
                table->seats[q].contestEligible = (q != p) && BridgeHasDiscardComplete(table, q, p);
                anyEligible |= table->seats[q].contestEligible;
            }
            NetworkPostDiscardInUse(table, p, anyEligible ? CONTESTABLE : NOT_IN_PLAY);
            BridgeTransitionTo(table, anyEligible ? WAIT_CONTEST : WAIT_DRAW);
            break;
        }

        case MATCH_REMOVE: {
            Match *m = &g->lastMatch;
            table->vacatedSlots = 0;

            for (int i = 0; i < table->expectCount; i++) {
                Expect *e = &table->expect[i];
                if (e->srcStand == m->playerIndex) {
                    PokajanDiscardAfterMatch(g, m->playerIndex, e->srcSlot);
                    table->vacatedSlots |= SLOT(e->srcSlot);
                }
            }

            if (m->useDiscardOf != -1) {
                NetworkPostDiscardInUse(table, m->useDiscardOf, NOT_IN_PLAY);
            }
            BridgeTransitionTo(table, MATCH_REPLENISH);
            break;
        }

        case MATCH_REPLENISH: {
            int p = table->matcherId;
            for (int i = 0; i < table->expectCount; i++) {
                Expect *e = &table->expect[i];
                if (e->dstSlot == SLOT_DRAWN) PokajanDraw(g, p, e->resolvedCard);
                else PokajanReplenish(g, p, e->resolvedCard, e->dstSlot);
            }

            NetworkPostDeckCount(table, g->cards);
            if (table->shortDeck) { BridgeTransitionTo(table, ENDED); break; }

            BridgeRefreshMatches(table, p);
            if (BridgeHasSelfComplete(table, p)) {
                BridgeTransitionTo(table, WAIT_CHAIN_DECISION);
            } else {
                BridgeFinishChain(table);
            }

            break;
        }
        
        default:
            // The rest of the states (decision) do not depend on expect events.
            break;
    }
}

static void BridgeAdvance(PokajanTable *table) {
    if (!BridgeExpectGroupComplete(table)) return;
    for (int i = 0; i < 4; i++) {
        memcpy(table->seats[i].shadow, table->seats[i].observed, sizeof(table->seats[i].shadow));
        memcpy(table->seats[i].shadowCard, table->seats[i].observedCard, sizeof(table->seats[i].shadowCard));
    }
    BridgeCompleteState(table);
}

void BridgeInitTable(PokajanTable *table) {
    struct mosquitto *mosq = table->mosq;
    bool online[4];
    for (int s = 0; s < 4; s++) online[s] = table->seats[s].online;

    memset(table, 0, sizeof(*table));

    table->mosq = mosq;
    for (int s = 0; s < 4; s++) table->seats[s].online = online[s];

    for (int s = 0; s < 4; s++) {
        for (int i = 0; i < 9; i++) {
            table->seats[s].observedCard[i] = table->seats[s].shadowCard[i] = EMPTY_CARD;
            SET_CARD_ID(table->seats[s].observed[i], EMPTY_CARD_ID);
            SET_CARD_ID(table->seats[s].shadow[i], EMPTY_CARD_ID);
        }
    }
    
    table->matcherId = -1;
    table->discarderId = -1;

    PokajanInit(&table->game);
    BridgePostCoins(table);
    NetworkPostCurrentTurn(table, table->game.turnIndex);
    NetworkPostDeckCount(table, table->game.cards);
    BridgeTransitionTo(table, WAIT_INITIAL_HANDS);
    BridgeUpdateActionRequired(table, true);
}

void BridgeRepublishSeat(PokajanTable *table, int s) {
    Seat *seat = &table->seats[s];
    NetworkPostCoins(table, s, table->game.players[s].coins);
    NetworkPostPlayerMatch(table, s, seat->matchCount, seat->matches);
    NetworkPostDiscardInUse(table, s, seat->discardLight);
    BridgePostSeatSlotStatus(table, s, true);
    BridgePostSeatActionRequired(table, s, true);
}

void BridgeRepublishState(PokajanTable *table) {
    Game *g = &table->game;

    int gens[4];
    for (int i = 0; i < 4; i++) gens[i] = g->generations[i];
    NetworkPostGenerations(table, gens);
    NetworkPostDeckCount(table, g->cards);
    NetworkPostCurrentTurn(table, table->state == ENDED ? -1 : g->turnIndex);
    NetworkPostCurrentMatcher(table, BridgeCurrentMatcher(table));

    if (table->state == ENDED) {
        int winners[4];
        int winnerCount = PokajanGetWinners(g, winners);
        NetworkPostWinners(table, winnerCount, winners);
    }

    for (int s = 0; s < 4; s++) BridgeRepublishSeat(table, s);
}

void BridgeOnSlotUpdate(PokajanTable *table, int standId, int from, int count, NfcId* cards) {
    for (int k = 0; k < count; k++) {
        int i = from + k;
        if (i < 0 || i >= 9) continue;

        SET_CARD_ID(table->seats[standId].observed[i], cards[k]);
        table->seats[standId].observedCard[i] = BridgeResolveCard(cards[k]);
    }

    BridgeSolveDiff(table);
    BridgePostSlotStatus(table, false);
    if (table->state != ENDED) BridgeAdvance(table);
}

static void BridgeResolveContest(PokajanTable *table) {
    Game *g = &table->game;

    if (g->contestants == 0) {
        NetworkPostDiscardInUse(table, table->discarderId, NOT_IN_PLAY);
        BridgeTransitionTo(table, WAIT_DRAW);
        return;
    }

    Match winner;
    PokajanResolveContestAndCommitDiscardMatch(g, &winner);

    BridgePostCoins(table);
    if (g->ended) { BridgeTransitionTo(table, ENDED); return; }

    NetworkPostDiscardInUse(table, table->discarderId, USED_IN_CONTEST);
    table->matcherId = winner.playerIndex;
    table->chainFromOwnTurn = false;
    NetworkPostCurrentMatcher(table, table->matcherId);
    BridgeTransitionTo(table, MATCH_REMOVE);
}

void BridgeOnDeclareAction(PokajanTable *table, int standId, DeclareAction action, int target) {
    if (table->seats[standId].mismatch) return; // eventually, send warning event to scene
    Game *g = &table->game;

    if (action == DECLARE && (target < 0 || target >= table->seats[standId].matchCount)) return;
    Match *m = (action == DECLARE) ? &table->seats[standId].matches[target] : NULL;

    switch (table->state) {
        case WAIT_SELF_DECISION:
        case WAIT_CHAIN_DECISION:
            if (standId != table->matcherId) return;
            if (action == SKIP) {
                if (table->state == WAIT_SELF_DECISION) {
                    BridgeTransitionTo(table, WAIT_DISCARD);
                } else {
                    BridgeFinishChain(table);
                }
                break;
            }
            if (!m->complete || m->useDiscardOf != -1) return;
            if (!PokajanCommitSelfMatch(g, standId, *m)) return;
            BridgePostCoins(table);
            if (g->ended) { BridgeTransitionTo(table, ENDED); break; }
            if (table->state == WAIT_SELF_DECISION) table->chainFromOwnTurn = true;
            NetworkPostCurrentMatcher(table, standId);
            BridgeTransitionTo(table, MATCH_REMOVE);
            break;

        case WAIT_CONTEST:
            if (!table->seats[standId].contestEligible || table->seats[standId].contestDecided) return;
            if (action == DECLARE) {
                if (!m->complete || m->useDiscardOf != table->discarderId) return;
                if (!PokajanDeclareContestOnDiscardMatch(g, standId, *m)) return; 
            }
            table->seats[standId].contestDecided = true;
            BridgeUpdateActionRequired(table, false);

            bool allDecided = true;
            for (int q = 0; q < 4; q++) {
                if (table->seats[q].contestEligible && !table->seats[q].contestDecided) allDecided = false;
            }

            if (allDecided) BridgeResolveContest(table);
            break;

        default:
            return;
    }

    BridgeAdvance(table);
    return;
}

void BridgeOnStatusUpdate(PokajanTable *table, int standId, bool online) {
    bool wasOnline = table->seats[standId].online;
    table->seats[standId].online = online;
    if (online && !wasOnline) BridgeRepublishSeat(table, standId);
}