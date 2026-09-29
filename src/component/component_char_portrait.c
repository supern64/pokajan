#include "component_char_mini_icon.h"
#include "../utils/misc.h"
#include <raylib.h>

static Texture2D Portraits[4];
static int PortraitSlots[4] = { -1, -1, -1, -1 };

void CharPortraitLoadIntoSlot(int memberId, int slot) {
    if (PortraitSlots[slot] != -1) return;
    Portraits[slot] = LoadTexture(TextFormat("assets/char_portrait/img_chr_full_2d_%05d.qoi", memberId));
    if (Portraits[slot].width != 0) {
        PortraitSlots[slot] = memberId;
        GenTextureMipmaps(&Portraits[slot]);
        SetTextureFilter(Portraits[slot], TEXTURE_FILTER_BILINEAR);
    }

}

void CharPortraitDrawRaw(int slot, Rectangle from, int x, int y, float scale, float rotation, Color tint) {
    DrawTexturePro(
        Portraits[slot],
        from,
        RECT_SCALE(x, y, from.width, from.height, scale),
        ANCHOR_5(from.width, from.height, scale),
        rotation,
        tint
    );
}

void CharPortraitEnsureLoaded(void) {
    for (int i = 0; i < 4; i++) {
        if (PortraitSlots[i] != -1) continue;
        CharPortraitLoadIntoSlot(1, i); // force load sora's portrait into all slots for debugging
    }
}

void CharPortraitUnloadSlot(int slot) {
    if (PortraitSlots[slot] == -1) return;
    UnloadTexture(Portraits[slot]);
    PortraitSlots[slot] = -1;
}

void CharPortraitUnloadAllSlots(void) {
    for (int i = 0; i < 4; i++) {
        if (PortraitSlots[i] == -1) continue;
        UnloadTexture(Portraits[i]);
        PortraitSlots[i] = -1;
    }
}