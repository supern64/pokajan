#include "network.h"
#include "network_internal.h"
#include "bridge.h"
#include "../utils/input.h"
#include <ctype.h>
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

    mosquitto_will_set(MosqInstance, "pokajan/hub/status", 1, "0", 1, true);
    
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
    static double lastAttempt = 0;
    int rc = mosquitto_loop(MosqInstance, 0, 1);

    if (rc == MOSQ_ERR_NO_CONN || rc == MOSQ_ERR_CONN_LOST) {
        double now = GetTime();
        if (now - lastAttempt >= 2.0) {
            lastAttempt = now;
            TraceLog(LOG_WARNING, "MQTT connection lost, reconnecting...");
            mosquitto_reconnect(MosqInstance); // NetworkOnConnect republishes on success
        }
    }
}

void NetworkShutdown() {
    if (MosqInstance != NULL) mosquitto_destroy(MosqInstance);
    MosqInstance = NULL;
    mosquitto_lib_cleanup();
}

// callback handlers
static void NetworkOnConnect(struct mosquitto *mosq, void *table, int reasonCode) {
    if (reasonCode != 0) {
        TraceLog(LOG_WARNING, TextFormat("MQTT connect refused (%d).", reasonCode));
        return;
    }

    mosquitto_publish(MosqInstance, NULL, "pokajan/hub/status", 1, "1", 1, true);
    
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
    BridgeRepublishState((PokajanTable*)table);
}

static bool NetworkReadNfcId(char* hexString, NfcId outId) {
    if (strlen(hexString) != 14) return false;
    for (int j = 0; j < 14; j++) {
        if (!isxdigit((unsigned char)hexString[j])) {
            return false;
        }
    }
    for (int j = 0; j < 7; j++) {
        sscanf(hexString + j*2, "%2hhx", &outId[j]);
    }
    return true;
}

// main event handler
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

    if (msg->payloadlen <= 0 || msg->payload == NULL) {
        TraceLog(LOG_WARNING, TextFormat("Empty payload on %s, ignoring.", msg->topic));
        return;
    }

    if (strcmp(action, "status") == 0) {
        int online;
        if (sscanf(msg->payload, "%d", &online) != 1) {
            TraceLog(LOG_WARNING, "Invalid payload received for topic status, ignoring.");
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
        char cardIdStr[7][15];
        NfcId hand[7];

        if (sscanf(msg->payload, "%14[^,],%14[^,],%14[^,],%14[^,],%14[^,],%14[^,],%14[^,]", cardIdStr[0], cardIdStr[1], cardIdStr[2], cardIdStr[3], cardIdStr[4], cardIdStr[5], cardIdStr[6]) != 7) {
            TraceLog(LOG_WARNING, "Invalid hand payload received, ignoring.");
            return;
        }

        for (int i = 0; i < 7; i++) {
            if (!NetworkReadNfcId(cardIdStr[i], hand[i])) {
                TraceLog(LOG_WARNING, "Invalid hand payload received, ignoring.");
                return;
            }
        }

        BridgeOnSlotUpdate(t, standId, 0, 7, hand);
    } else if (strcmp(action, "drawn") == 0) {
        NfcId drawn;

        if (!NetworkReadNfcId(msg->payload, drawn)) {
            TraceLog(LOG_WARNING, "Invalid drawn payload received, ignoring.");
            return;
        }

        BridgeOnSlotUpdate(t, standId, 7, 1, &drawn);
    } else if (strcmp(action, "discard") == 0) {
        NfcId discard;

        if (!NetworkReadNfcId(msg->payload, discard)) {
            TraceLog(LOG_WARNING, "Invalid discard payload received, ignoring.");
            return;
        }

        BridgeOnSlotUpdate(t, standId, 8, 1, &discard);
    } else if (strcmp(action, "action") == 0) {
        int decAction;
        int target = -1;
        if (sscanf(msg->payload, "%d,%d", &decAction, &target) < 1 || (decAction != DECLARE && decAction != SKIP))  {
            TraceLog(LOG_WARNING, "Invalid action payload received, ignoring.");
            return;
        }

        BridgeOnDeclareAction(t, standId, decAction, target);
    } else if (strcmp(action, "button") == 0) {
        uint8_t button, type;
        if (sscanf(msg->payload, "%hhu,%hhu", &button, &type) != 2 || (button != 0 && button != 1) || (type != 0 && type != 1)) return;
        if (type == 1) {
            InputPress(standId, button);
        } else {
            InputRelease(standId, button);
        }
    } else {
        TraceLog(LOG_WARNING, TextFormat("Invalid topic %s received, ignoring.", action));
    }
}

// posting information
void NetworkPostCoins(PokajanTable *table, int standId, int coins) {
    char nCoins[5];
    snprintf(nCoins, 5, "%d", coins);
    mosquitto_publish(table->mosq, NULL, TextFormat("pokajan/stand/%d/coins", standId), strlen(nCoins), nCoins, 1, true);
}

void NetworkPostPlayerMatch(PokajanTable *table, int standId, int nMatch, Match matches[POKAJAN_MAX_MATCHES]) {
    int bSize = nMatch * 21 + 1;
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
                if (IS_SAME_CARD(matches[i].matchInHand[j], table->game.players[standId].hand[k])) {
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
    mosquitto_publish(table->mosq, NULL, TextFormat("pokajan/stand/%d/discard_in_use", standId), 1, TextFormat("%d", state), 1, true);
}

void NetworkPostMatchActionRequired(PokajanTable *table, int standId, bool required) {
    mosquitto_publish(table->mosq, NULL, TextFormat("pokajan/stand/%d/match_action_required", standId), 1, required ? "1" : "0", 1, true);
}

void NetworkPostError(PokajanTable *table, int standId, uint16_t slotMask, uint16_t typeMask) {
    char errorStr[18];
    errorStr[0] = '\0';

    for (int i = 0; i < 9; i++) {
        if (i != 0) strncat(errorStr, ",", 18 - strlen(errorStr) - 1);
        if (slotMask >> i & 1) {
            strncat(errorStr, (typeMask >> i & 1) ? "2" : "1", 18 - strlen(errorStr) - 1);
        } else {
            strncat(errorStr, "0", 18 - strlen(errorStr) - 1);
        }
    }

    mosquitto_publish(table->mosq, NULL, TextFormat("pokajan/stand/%d/error", standId), strlen(errorStr), errorStr, 1, true);
}

void NetworkPostCurrentTurn(PokajanTable *table, int turnIndex) {
    char currentTurnStr[3];
    snprintf(currentTurnStr, 3, "%d", turnIndex);
    mosquitto_publish(table->mosq, NULL, "pokajan/hub/game/current_turn", strlen(currentTurnStr), currentTurnStr, 1, true);
}

void NetworkPostCurrentMatcher(PokajanTable *table, int matcherIndex) {
    char matcherStr[3];
    snprintf(matcherStr, 3, "%d", matcherIndex);
    mosquitto_publish(table->mosq, NULL, "pokajan/hub/game/current_match", strlen(matcherStr), matcherStr, 1, true);
}

void NetworkPostWinners(PokajanTable *table, int winnerCount, int winnerIdx[4]) {
    char winnersStr[8];
    winnersStr[0] = '\0';

    for (int i = 0; i < winnerCount; i++) {
        if (i != 0) strncat(winnersStr, ",", 8 - strlen(winnersStr) - 1);
        strncat(winnersStr, TextFormat("%d", winnerIdx[i]), 8 - strlen(winnersStr) - 1);
    }

    mosquitto_publish(table->mosq, NULL, "pokajan/hub/game/winners", strlen(winnersStr), winnersStr, 1, true);
}

void NetworkPostGenerations(PokajanTable *table, int generations[4]) {
    char genList[12];
    snprintf(genList, 12, "%d,%d,%d,%d", generations[0], generations[1], generations[2], generations[3]);
    mosquitto_publish(table->mosq, NULL, "pokajan/hub/debug/generations", strlen(genList), genList, 1, true);
}

void NetworkPostDeckCount(PokajanTable *table, int count) {
    char nDeckCount[4];
    snprintf(nDeckCount, 4, "%d", count);
    mosquitto_publish(table->mosq, NULL, "pokajan/hub/debug/deck_count", strlen(nDeckCount), nDeckCount, 1, true);
}