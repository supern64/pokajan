#ifndef COMPONENT_HUD_H
#define COMPONENT_HUD_H

#include "../pokajan_core/pokajan.h"
#include "../pokajan_core/cards.h"

void HUDLoad(void);
void HUDDrawPlayers(Player players[4]);
void HUDDrawGenIndicator(Generation generation, int x, int y, float scale, float rotation);
void HUDDrawGenIndicators(Generation generations[4]);
void HUDUnload(void);

#endif