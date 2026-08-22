#include "component_hud.h"
#include <raylib.h>

static Texture2D GameAtlas;

void HUDLoad(void) {
    GameAtlas = LoadTexture("assets/game_atlas.png");
}

void HUDDrawGenIndicator(Generation generation, int x, int y, float scale, float rotation) {
    Rectangle atlasLocation;
    switch (generation) {
        case GEN_0:
            atlasLocation = (Rectangle){ 348, 1, 43, 59 };
            break;
        case GEN_1:
            atlasLocation = (Rectangle){ 393, 1, 34, 57 };
            break;
        case GEN_2:
            atlasLocation = (Rectangle){ 429, 1, 42, 58 };
            break;
        case GAMERS:
            atlasLocation = (Rectangle){ 278, 1, 68, 61 };
            break;
        case GEN_3:
            atlasLocation = (Rectangle){ 473, 1, 44, 58 };
            break;
        case GEN_4:
            atlasLocation = (Rectangle){ 519, 1, 48, 57 };
            break;
        case GEN_5:
            atlasLocation = (Rectangle){ 569, 1, 42, 58 };
            break;
        case HOLOX:
            atlasLocation = (Rectangle){ 613, 1, 45, 55 };
            break;
        case MYTH:
            atlasLocation = (Rectangle){ 887, 1, 71, 60 };
            break;
        case PROMISE:
            atlasLocation = (Rectangle){ 960, 1, 57, 55 };
            break;
        case ADVENT:
            atlasLocation = (Rectangle){ 205, 1, 71, 57 };
            break;
        case ID_GEN_1:
            atlasLocation = (Rectangle){ 660, 1, 72, 57 };
            break;
        case ID_GEN_2:
            atlasLocation = (Rectangle){ 734, 1, 74, 61 };
            break;
        case ID_GEN_3:
            atlasLocation = (Rectangle){ 810, 1, 75, 61 };
            break;
        case REGLOSS:
            atlasLocation = (Rectangle){ 1019, 1, 63, 56 };
            break;
    }
    // draw centered to X and Y
    DrawTexturePro(
        GameAtlas,
        atlasLocation,
        (Rectangle){ x, y, atlasLocation.width * scale, atlasLocation.height * scale },
        (Vector2) { 0, 0 },
        rotation,
        WHITE
    );
}

// specifically for use with the central deck
void HUDDrawGenIndicators(Generation generations[4]) {
    int slot = 0;
    for (slot = 0; slot < 2; slot++) {
        HUDDrawGenIndicator(generations[slot], 240, 290 + slot * 240, 1.0, 0.0);
        HUDDrawGenIndicator(generations[slot], 300 + GENERATION_MEMBER_COUNT[generations[slot]] * 100, 480 + slot * 240, 1.0, 180.0);
	}

	for (slot = 0; slot < 2; slot++) {
		HUDDrawGenIndicator(generations[slot+2], 840, 290 + slot * 240, 1.0, 0.0);
        HUDDrawGenIndicator(generations[slot+2], 900 + GENERATION_MEMBER_COUNT[generations[slot+2]] * 100, 480 + slot * 240, 1.0, 180.0);
	}
}

void HUDUnload(void) {
    UnloadTexture(GameAtlas);
}