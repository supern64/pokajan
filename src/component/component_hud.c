#include "component_hud.h"
#include <raylib.h>
#include <stdio.h>
#include "../utils/text.h"

#define TABLE_BLEND (Color){ 0, 0, 0, 64 }

static Texture2D GameAtlas;

void HUDLoad(void) {
    GameAtlas = LoadTexture("assets/game_atlas.png");
    GenTextureMipmaps(&GameAtlas);
    SetTextureFilter(GameAtlas, TEXTURE_FILTER_BILINEAR);
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

    DrawTexturePro(
        GameAtlas,
        atlasLocation,
        (Rectangle){ x, y, atlasLocation.width * scale, atlasLocation.height * scale },
        (Vector2){ 0, 0 },
        rotation,
        WHITE
    );
}

// specifically for use with the central deck
void HUDDrawGenIndicators(Generation generations[4]) {
    int slot = 0;
    for (slot = 0; slot < 2; slot++) {
        HUDDrawGenIndicator(generations[slot], 240, 290 + slot * 240, 1.0, 0.0);
        HUDDrawGenIndicator(generations[slot], (GENERATION_MEMBER_COUNT[generations[slot]] == 4 ? 760 : 800), 480 + slot * 240, 1.0, 180.0);
	}

	for (slot = 0; slot < 2; slot++) {
		HUDDrawGenIndicator(generations[slot+2], 840, 290 + slot * 240, 1.0, 0.0);
        HUDDrawGenIndicator(generations[slot+2], (GENERATION_MEMBER_COUNT[generations[slot+2]] == 4 ? 1360 : 1400), 480 + slot * 240, 1.0, 180.0);
	}
}

static void HUDDrawCoin(int x, int y, float scale, float rotation) {
    DrawTexturePro(
        GameAtlas,
        (Rectangle){ 1, 1, 202, 200 },
        (Rectangle){ x, y, 202 * scale, 200 * scale },
        (Vector2){ 0, 0 },
        rotation,
        WHITE
    );
}

static void HUDDrawCoinNumber(int coins, int x, int y) {
    if (coins > 9999) coins = 9999;
    if (coins < -999) coins = -999;
    char coinText[5];
    snprintf(coinText, 5, "%d", coins);
    Vector2 size = MeasureTextEx(*GetFocusFont(), coinText, 50, 1.0);
    DrawTextPro(*GetFocusFont(), coinText, (Vector2){ x, y }, (Vector2){ size.x, size.y / 2 }, 0.0, 50, 1.0, WHITE);
}

// 0 - 1st place, 3 - 4th place
static void HUDDrawPlace(int place, int x, int y, float scale, float rotation) {
    Rectangle atlasLocation;
    switch (place) {
        case 0:
            atlasLocation = (Rectangle){ 1, 866, 326, 285 };
            break;
        case 1:
            atlasLocation = (Rectangle){ 329, 866, 375, 292 };
            break;
        case 2:
            atlasLocation = (Rectangle){ 706, 866, 365, 295 };
            break;
        case 3:
            atlasLocation = (Rectangle){ 1073, 866, 374, 286 };
            break;
    }

    DrawTexturePro(
        GameAtlas,
        atlasLocation,
        (Rectangle){ x, y, atlasLocation.width * scale, atlasLocation.height * scale },
        (Vector2){ 0, 0 },
        rotation,
        WHITE
    );
}

static void HUDCalculatePlayerRank(Player players[4], int outRanks[4]) {
    int order[4] = {0, 1, 2, 3};

    // Sort player indices by coins descending (simple insertion sort, only 4 elements)
    for (int i = 1; i < 4; i++) {
        int key = order[i];
        int keyCoins = players[key].coins;
        int j = i - 1;
        while (j >= 0 && players[order[j]].coins < keyCoins) {
            order[j + 1] = order[j];
            j--;
        }
        order[j + 1] = key;
    }

    // Assign ranks, handling ties (equal coins => equal rank)
    outRanks[order[0]] = 0;
    for (int i = 1; i < 4; i++) {
        if (players[order[i]].coins == players[order[i - 1]].coins) {
            outRanks[order[i]] = outRanks[order[i - 1]];
        } else {
            outRanks[order[i]] = i;
        }
    }
}


void HUDDrawPlayers(Player players[4]) {
    int ranks[4];
    HUDCalculatePlayerRank(players, ranks);


    // P1
    // P2

    // P3
    DrawCircle(786, 112, 60, TABLE_BLEND);
    DrawRing((Vector2){ 786, 112 }, 60, 70, 0, 360, 30, YELLOW);
    DrawRectangleRounded((Rectangle) { 870, 60, 300, 100 }, 1.5, 30, TABLE_BLEND);
    DrawTextPro(*GetFocusFont(), "P3", (Vector2){ 750, 75 }, (Vector2){ 0, 0 }, 0.0, 70, 1.0, WHITE);
    HUDDrawCoin(900, 90, 0.2, 0.0);
    HUDDrawCoinNumber(players[2].coins, 1050, 109);
    HUDDrawPlace(ranks[2], 1070, 75, 0.2, 0.0);
    
    // P4
    
    
}

void HUDUnload(void) {
    UnloadTexture(GameAtlas);
}