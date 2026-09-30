#include "component_hud.h"
#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include "../utils/text.h"
#include "../utils/misc.h"

static Texture2D GameAtlas;

void HUDLoad(void) {
    GameAtlas = LoadTexture("assets/game_atlas.qoi");
    GenTextureMipmaps(&GameAtlas);
    SetTextureFilter(GameAtlas, TEXTURE_FILTER_BILINEAR);
}

// gen indicators and central deck overlay

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
        RECT_SCALE(x, y, atlasLocation.width, atlasLocation.height, scale),
        ANCHOR_7,
        rotation,
        WHITE
    );
}

void HUDDrawCoin(int x, int y, float scale, float rotation, int alpha) {
    DrawTexturePro(
        GameAtlas,
        (Rectangle){ 1, 1, 202, 200 },
        RECT_SCALE(x, y, 202, 200, scale),
        ANCHOR_7,
        rotation,
        WHITE_ALPHA(alpha)
    );
}

void HUDDrawCoinNumber(int coins, int x, int y, float rotation, Color color) {
    if (coins > 9999) coins = 9999;
    if (coins < -999) coins = -999;
    char coinText[5];
    snprintf(coinText, 5, "%d", coins);
    Vector2 size = MeasureTextEx(*GetFocusFont(), coinText, 50, 1.0);
    DrawTextPro(*GetFocusFont(), coinText, (Vector2){ x, y }, ANCHOR_6(size.x, size.y, 1), rotation, 50, 1.0, color);
}

void HUDDrawPlace(int place, int x, int y, float scale, float rotation, int alpha) {
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
        RECT_SCALE(x, y, atlasLocation.width, atlasLocation.height, scale),
        ANCHOR_5(atlasLocation.width, atlasLocation.height, scale),
        rotation,
        WHITE_ALPHA(alpha)
    );
}

void HUDCalculatePlayerRank(const Player players[4], int outRanks[4]) {
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

void HUDDrawRectangleRoundedRotated(Rectangle rec, float roundness, int segments, float rotation, Color color) {
    // rec.x/rec.y here should be the rectangle's CENTER, not top-left
    rlPushMatrix();
        rlTranslatef(rec.x, rec.y, 0.0f);
        rlRotatef(rotation, 0.0f, 0.0f, 1.0f);

        // Draw centered on the new local origin (0,0), so offset by -w/2, -h/2
        Rectangle localRec = { -rec.width / 2, -rec.height / 2, rec.width, rec.height };
        DrawRectangleRounded(localRec, roundness, segments, color);
    rlPopMatrix();
}

void HUDDrawRectangleRoundedLineRotated(Rectangle rec, float roundness, int segments, float rotation, float lineThick, Color color) {
    rlPushMatrix();
        rlTranslatef(rec.x, rec.y, 0.0f);
        rlRotatef(rotation, 0.0f, 0.0f, 1.0f);

        Rectangle localRec = { -rec.width / 2, -rec.height / 2, rec.width, rec.height };
        DrawRectangleRoundedLinesEx(localRec, roundness, segments, lineThick, color);
    rlPopMatrix();
}

void HUDDrawPokajanLogo(int x, int y, float scale, float rotation, Color tint) {
    DrawTexturePro(
        GameAtlas,
        (Rectangle){ 1, 203, 1355, 661 },
        RECT_SCALE(x, y, 1355, 661, scale),
        ANCHOR_7,
        rotation,
        tint
    );
}

void HUDUnload(void) {
    UnloadTexture(GameAtlas);
}