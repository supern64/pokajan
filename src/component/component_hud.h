#ifndef COMPONENT_HUD_H
#define COMPONENT_HUD_H

#include "../pokajan_core/pokajan.h"
#include "../pokajan_core/cards.h"
#include "../network/bridge.h"

void HUDLoad(void);

void HUDDrawGenIndicator(Generation generation, int x, int y, float scale, float rotation);
void HUDDrawCoin(int x, int y, float scale, float rotation, int alpha);
void HUDDrawCoinNumber(int coins, int x, int y, float rotation, Color color);
void HUDDrawPlace(int place, int x, int y, float scale, float rotation, int alpha);
void HUDCalculatePlayerRank(const Player players[4], int outRanks[4]);
void HUDDrawRectangleRoundedRotated(Rectangle rec, float roundness, int segments, float rotation, Color color);
void HUDDrawRectangleRoundedLineRotated(Rectangle rec, float roundness, int segments, float rotation, float lineThick, Color color);
void HUDDrawPokajanLogo(int x, int y, float scale, float rotation, Color tint);

void HUDUnload(void);

#endif