#ifndef COMPONENT_HUD_H
#define COMPONENT_HUD_H

// #include "pokajan.h"
#include "../pokajan_core/cards.h"

void HUDLoad(void);
// void HUDRenderPlayers(Player players[4]);
void HUDDrawGenIndicator(Generation generation, int x, int y, float scale, float rotation);
void HUDDrawGenIndicators(Generation generations[4]);
void HUDUnload(void);

#endif