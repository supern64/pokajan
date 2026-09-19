#ifndef COMPONENT_CHAR_MINI_ICON_H
#define COMPONENT_CHAR_MINI_ICON_H

#include "../pokajan_core/cards.h"

void CharMiniIconLoad(void);
void CharMiniIconDrawRaw(Generation generation, int memSlot, int x, int y, float scale, float rotation);
void CharMiniIconUnload(void);

#endif