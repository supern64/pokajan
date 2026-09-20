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

int BridgeLoadCardTable() {
    FILE *f = fopen("card_table.csv", "r");
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
    return i;
}

Card BridgeResolveCard(NfcId id) {
    if (memcmp(id, (NfcId){ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }, 7) == 0) return EMPTY_CARD;
    for (int i = 0; i < loadedCount; i++) {
        if (memcmp(CTable[i].id, id, 7) == 0) {
            return CTable[i].card;
        }
    }
    return EMPTY_CARD;
}

void BridgeInitTable(PokajanTable *table) {
    PokajanInit(&table->game);

    table->allReady = false;
    for (int i = 0; i < 4; i++) {
        table->seats[i] = (Seat){
            .online = false,
            .handReady = false,
            .state = WAIT_DRAW,
            .member = (MemberSlot){ 0, 1, 0, 0 }, // Tokino Sora
            .hand = { EMPTY_CARD_ID, EMPTY_CARD_ID, EMPTY_CARD_ID, EMPTY_CARD_ID, EMPTY_CARD_ID, EMPTY_CARD_ID, EMPTY_CARD_ID },
            .drawnSlot = EMPTY_CARD_ID,
            .discardSlot = EMPTY_CARD_ID
        };
    }
}

void BridgeOnHandUpdate(PokajanTable *table, int standId, NfcId hand[7]) {
    if (!table->allReady) {
        // not all cards have been initialized -> see if every stand is ready then fire allReady
        // no wrong state, so no need to check cards yet

        bool isReady = true;
        for (int i = 0; i < 7; i++) {
            memcpy(table->seats[standId].hand[i], hand[i], 7);
            isReady = memcmp(hand[i], (NfcId)EMPTY_CARD_ID, 7) != 0;
        }
        table->seats[standId].handReady = isReady;

        bool allReady = true;
        for (int i = 0; i < 4; i++) {
            if (!table->seats[i].handReady) {
                allReady = false;
                break;
            }
        }
        table->allReady = allReady;

        if (allReady) {
            for (int i = 0; i < 4; i++) {
                Card engineHand[7];
                for (int j = 0; j < 7; j++) {
                    engineHand[j] = BridgeResolveCard(table->seats[i].hand[j]);
                }
                PokajanSetInitialHand(&table->game, i, engineHand);
            }

            for (int i = 0; i < 4; i++) {
                Match matches[POKAJAN_MAX_MATCHES];
                int nMatch = PokajanCheckMatches(&table->game, i, matches);
                if (nMatch > 0) NetworkPostPlayerMatch(table, i, nMatch, matches);
            }
        }
    } else {
        // process card difference (removal/inserts)
    }
}

void BridgeOnDrawnUpdate(PokajanTable *table, int standId, NfcId drawn) {

}

void BridgeOnDiscardUpdate(PokajanTable *table, int standId, NfcId discard) {

}

void BridgeOnDeclareAction(PokajanTable *table, int standId, DeclareAction action, int target) {

}

void BridgeOnStatusUpdate(PokajanTable *table, int standId, bool online) {
    table->seats[standId].online = online;
}