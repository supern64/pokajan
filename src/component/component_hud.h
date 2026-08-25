#ifndef COMPONENT_HUD_H
#define COMPONENT_HUD_H

#include "../pokajan_core/pokajan.h"
#include "../pokajan_core/cards.h"

void HUDLoad(void);
void HUDDrawPlayers(Player players[4], int turnIndex);
void HUDDrawGenIndicator(Generation generation, int x, int y, float scale, float rotation);
void HUDDrawGenIndicators(Generation generations[4]);
void HUDDrawPokajanAnim(void);
void HUDInitPokajanAnim(int playerIndex);
void HUDUpdatePokajanAnim(void);
void HUDUnload(void);

#endif