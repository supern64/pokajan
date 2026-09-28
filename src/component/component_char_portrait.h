#ifndef COMPONENT_CHAR_PORTRAIT_H
#define COMPONENT_CHAR_PORTRAIT_H

#include <raylib.h>

void CharPortraitLoadIntoSlot(int memberId, int slot);
void CharPortraitDrawRaw(int slot, Rectangle from, int x, int y, float scale, float rotation, Color tint);
void CharPortraitEnsureLoaded(void);
void CharPortraitUnloadSlot(int slot);
void CharPortraitUnloadAllSlots(void);

#endif