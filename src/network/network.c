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
    BridgePostGameState((PokajanTable*)table);
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
        // TODO
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

