#ifndef NETWORK_INTERNAL_H
#define NETWORK_INTERNAL_H

#include <mosquitto.h>

static void NetworkOnConnect(struct mosquitto *mosq, void *table, int reasonCode);
static void NetworkOnMessage(struct mosquitto *mosq, void *table, const struct mosquitto_message *msg);
#endif