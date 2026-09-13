#include "bridge.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <mosquitto.h>

static CardTable CTable;
static int loadedCount = 0;

int BridgeLoadCardTable() {
    FILE *f = fopen("card_table.csv", "r");
    char line[32];
    int i = 0;
    while (fgets(line, sizeof(line), f) && i < MAX_CARDS) {
        char uid_hex[15];
        int id;
        Generation gen;
        Variant var;
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
            .state = WAIT_DRAW,
            .characterId = 1,
            .hand = { EMPTY_CARD_ID, EMPTY_CARD_ID, EMPTY_CARD_ID, EMPTY_CARD_ID, EMPTY_CARD_ID, EMPTY_CARD_ID, EMPTY_CARD_ID },
            .drawnSlot = EMPTY_CARD_ID,
            .discardSlot = EMPTY_CARD_ID
        };
    }
}

void BridgeOnStatusUpdate(PokajanTable *table, int standId, bool online) {
    table->seats[standId].online = online;
}