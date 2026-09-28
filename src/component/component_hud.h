#ifndef COMPONENT_HUD_H
#define COMPONENT_HUD_H

#include "../pokajan_core/pokajan.h"
#include "../pokajan_core/cards.h"
#include "../network/bridge.h"

void HUDLoad(void);

// for main game scene
void HUDDrawSeats(PokajanTable* table);
void HUDDrawGenIndicator(Generation generation, int x, int y, float scale, float rotation);
void HUDDrawGenIndicators(Generation generations[4]);

// for overlay
void HUDDrawPokajanLogo(int x, int y, float scale, float rotation, Color tint);

void HUDUnload(void);

#endif