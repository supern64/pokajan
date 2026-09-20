#include "network.h"
#include "network_internal.h"
#include "bridge.h"
#include "../utils/input.h"
#include <stdio.h>
#include <string.h>
#include <mosquitto.h>
#include <raylib.h>

struct mosquitto *MosqInstance = NULL;

bool NetworkInit(PokajanTable* table) {
    mosquitto_lib_init();

    MosqInstance = mosquitto_new("hub", true, table);
    if (MosqInstance == NULL) {
        TraceLog(LOG_FATAL, "Failed to initialize MQTT instance. Will exit.");
        // unreachable
        return false;
    }
    table->mosq = MosqInstance;

    mosquitto_connect_callback_set(MosqInstance, NetworkOnConnect);
    mosquitto_message_callback_set(MosqInstance, NetworkOnMessage);

    mosquitto_will_set(MosqInstance, "pokajan/hub/status", 2, "0", 1, true);
    
    TraceLog(LOG_INFO, "Attempting to connect...");
    int res = mosquitto_connect(MosqInstance, DEFAULT_IP, DEFAULT_PORT, 60);

    if (res != MOSQ_ERR_SUCCESS) {
        TraceLog(LOG_FATAL, "Failed to connect to MQTT instance. Will exit.");
        // unreachable
        return false;
    } else {
        TraceLog(LOG_INFO, "Successfully connected to MQTT instance.");
        return true;
    }
}

void NetworkLoop() {
    mosquitto_loop(MosqInstance, 0, 1);
}

void NetworkShutdown() {
    if (MosqInstance != NULL) mosquitto_destroy(MosqInstance);
    MosqInstance = NULL;
    mosquitto_lib_cleanup();
}

// callback handlers
static void NetworkOnConnect(struct mosquitto *mosq, void *table, int reasonCode) {
    mosquitto_publish(MosqInstance, NULL, "pokajan/hub/status", 2, "1", 1, true);
    
    char* subTopics[] = {
        "pokajan/stand/+/status",
        "pokajan/stand/+/hand",
        "pokajan/stand/+/drawn",
        "pokajan/stand/+/discard",
        "pokajan/stand/+/action",
        "pokajan/stand/+/button"
    };
    
    mosquitto_subscribe_multiple(MosqInstance, NULL, 6, subTopics, 2, 0, NULL);

    // broadcast initial game information
    NetworkPostGameState((PokajanTable*)table);
}

static void NetworkOnMessage(struct mosquitto *mosq, void *table, const struct mosquitto_message *msg) {
    PokajanTable* t = (PokajanTable*)table;

    int standId;
    char action[8];

    if (sscanf(msg->topic, "pokajan/stand/%d/%7s", &standId, action) != 2) {
        TraceLog(LOG_WARNING, "Invalid topic format received, ignoring.");
        return;
    }

    if (standId < 0 || standId > 3) {
        TraceLog(LOG_WARNING, TextFormat("Invalid stand ID %d referenced, ignoring.", standId));
        return;
    }

    if (strcmp(action, "status") == 0) {
        int online;
        if (sscanf(msg->payload, "%d", &online) != 1) {
            TraceLog(LOG_WARNING, TextFormat("Invalid payload received for topic online, ignoring.", action));
            return;
        }
        BridgeOnStatusUpdate(t, standId, online == 1);
        if (online == 0) { // from LWT
            InputRelease(standId, POKAJAN);
            InputRelease(standId, SKIP_CYCLE);
        }
        return;
    }


    if (!t->seats[standId].online) return; // reject all offline seats
    
    if (strcmp(action, "hand") == 0) {
        NfcId hand[7];
        
    } else if (strcmp(action, "drawn") == 0) {
        // TODO
    } else if (strcmp(action, "discard") == 0) {
        // TODO
    } else if (strcmp(action, "action") == 0) {
        // TODO
    } else if (strcmp(action, "button") == 0) {
        uint8_t button, type;
        sscanf(msg->payload, "%hhu,%hhu", &button, &type);
        if (type == 1) {
            InputPress(standId, button);
        } else {
            InputRelease(standId, button);
        }
    } else {
        TraceLog(LOG_WARNING, TextFormat("Invalid topic %s received, ignoring.", action));
    }
}

void NetworkPostGameState(PokajanTable *table) {
    for (int i = 0; i < 4; i++) {
        char nCoins[5];
        snprintf(nCoins, 5, "%d", table->game.players[i].coins);
        mosquitto_publish(table->mosq, NULL, TextFormat("pokajan/stand/%d/coins", i), strlen(nCoins), nCoins, 1, true);
    }

    mosquitto_publish(table->mosq, NULL, "pokajan/hub/game/current_turn", 2, TextFormat("%d", table->game.turnIndex), 1, true);

    char genList[12];
    snprintf(genList, 12, "%hhu,%hhu,%hhu,%hhu", table->game.generations[0], table->game.generations[1], table->game.generations[2], table->game.generations[3]);
    mosquitto_publish(table->mosq, NULL, "pokajan/hub/debug/generations", strlen(genList), genList, 1, true);

    char nDeckCount[4];
    snprintf(nDeckCount, 4, "%d", table->game.cards);
    mosquitto_publish(table->mosq, NULL, "pokajan/hub/debug/deck_count", strlen(nDeckCount), nDeckCount, 1, true);
}

void NetworkPostPlayerMatch(PokajanTable *table, int standId, int nMatch, Match matches[POKAJAN_MAX_MATCHES]) {
    int bSize = nMatch * 15 + 1;
    char matchArray[bSize];

    matchArray[0] = '\0';
    // for every match...
    for (int i = 0; i < nMatch; i++) {
        if (IS_EMPTY_MATCH(matches[i])) continue;
        bool used[8] = { false };
        int j = 0;
        // loop through each card used in the match...
        while (j < 5 && !IS_EMPTY_CARD(matches[i].matchInHand[j])) {
            // and check which card in hand matches.
            int inHand = -1;
            for (int k = 0; k < 7; k++) {
                if (used[k]) continue;
                if (IS_SAME_CARD(matches[i].matchInHand[j], table->game.players[standId].hand[i])) {
                    inHand = k;
                    used[k] = true;
                    break;
                }
            }
            if (inHand == -1 && !used[7] && IS_SAME_CARD(matches[i].matchInHand[j], table->game.players[standId].drawnSlot)) {
                inHand = 7;
                used[7] = true;
            }
            // player sim handles -1 gracefully.
            
            char slotStr[4];
            snprintf(slotStr, 4, (j+1 < 5 && !IS_EMPTY_CARD(matches[i].matchInHand[j+1])) ? "%d," : "%d", inHand);
            strncat(matchArray, slotStr, bSize - strlen(matchArray) - 1);

            j++;
        }

        char rewardStr[7];
        snprintf(rewardStr, 7, ":%d;", matches[i].reward);
        strncat(matchArray, rewardStr, bSize - strlen(matchArray) - 1);
    }

    mosquitto_publish(table->mosq, NULL, TextFormat("pokajan/stand/%d/matches", standId), strlen(matchArray), matchArray, 1, true);
}

void NetworkPostDiscardInUse(PokajanTable *table, int standId, DiscardLightState state) {
    mosquitto_publish(table->mosq, NULL, TextFormat("pokajan/stand/%d/discard_in_use", standId), 2, TextFormat("%hhu", state), 1, true);
}

void NetworkPostMatchActionRequired(PokajanTable *table, int standId, bool required) {
    mosquitto_publish(table->mosq, NULL, TextFormat("pokajan/stand/%d/match_action_required", standId), 2, required ? "1" : "0", 1, true);
}
