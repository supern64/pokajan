#ifndef NETWORK_H
#define NETWORK_H

#include <stdbool.h>
#include "bridge.h"

#define DEFAULT_IP "127.0.0.1"
#define DEFAULT_PORT 1883

bool NetworkInit(PokajanTable* table);
void NetworkLoop();
void NetworkShutdown();

#endif