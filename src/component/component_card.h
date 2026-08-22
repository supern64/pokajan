#ifndef COMPONENT_CARD_H
#define COMPONENT_CARD_H

#include "../pokajan_core/cards.h"

// Loads 4 card atlas textures for 4 generations.
void CardLoad(Generation gen_array[4]);

// Unloads all card textures.
void CardUnload();

// Draws a specific card to the screen.
void CardDraw(Card card, int x, int y, float scale);

// Draws a specific card to the screen.
void CardDrawRaw(int slot, int memIndex, Variant variant, int x, int y, float scale);

#endif