#include "component_char_mini_icon.h"
#include "../pokajan_core/cards.h"
#include "../utils/misc.h"
#include <raylib.h>

static Texture2D CharMiniIconAtlas;

void CharMiniIconLoad(void) {
    CharMiniIconAtlas = LoadTexture("assets/char_mini_icon_atlas.png");
    GenTextureMipmaps(&CharMiniIconAtlas);
    SetTextureFilter(CharMiniIconAtlas, TEXTURE_FILTER_BILINEAR);
}

// anchor is centered for easier rotation
void CharMiniIconDrawRaw(Generation generation, int memSlot, int x, int y, float scale, float rotation) {
    Rectangle atlasLocation = (Rectangle){ memSlot * 128, generation * 128, 128, 128 };
    Rectangle finalLocation = (Rectangle){ x, y, 128 * scale, 128 * scale };
    DrawTexturePro(
        CharMiniIconAtlas,
        atlasLocation,
        finalLocation,
        ANCHOR_5(128, 128, scale),
        rotation,
        WHITE
    );
}

void CharMiniIconUnload(void) {
    UnloadTexture(CharMiniIconAtlas);
}