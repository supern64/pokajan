#ifndef COMPONENT_CHAR_PORTRAIT_H
#define COMPONENT_CHAR_PORTRAIT_H

#include <raylib.h>

void CharPortraitLoadIntoSlot(int memberId, int slot);
void CharPortraitDrawRaw(int slot, Rectangle from, int x, int y, float scale, float rotation);
void CharPortraitUnloadAllSlots(void);

#endif