#include "component_char_mini_icon.h"
#include "../utils/misc.h"
#include <raylib.h>

static Texture2D Portraits[4];
static int PortraitSlots[4] = { -1, -1, -1, -1 };

void CharPortraitLoadIntoSlot(int memberId, int slot) {
    if (PortraitSlots[slot] != -1) return;
    Portraits[slot] = LoadTexture(TextFormat("assets/char_portrait/img_chr_full_2d_%05d.png", memberId));
    if (Portraits[slot].width != 0) PortraitSlots[slot] = memberId;
}

void CharPortraitDrawRaw(int slot, Rectangle from, int x, int y, float scale, float rotation) {
    DrawTexturePro(
        Portraits[slot],
        from,
        (Rectangle){ x, y, from.width * scale, from.height * scale },
        ANCHOR_5(from.width, from.height, scale),
        rotation,
        WHITE
    );
}

void CharPortraitUnloadAllSlots(void) {
    for (int i = 0; i < 4; i++) {
        if (PortraitSlots[i] == -1) continue;
        UnloadTexture(Portraits[i]);
        PortraitSlots[i] = -1;
    }
}