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
    GameAtlas = LoadTexture("assets/game_atlas.png");
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
        (Rectangle){ x, y, atlasLocation.width * scale, atlasLocation.height * scale },
        ANCHOR_7,
        rotation,
        WHITE
    );
}

void HUDDrawGenIndicators(Generation generations[4]) {
    int slot = 0;
    for (slot = 0; slot < 2; slot++) {
        HUDDrawGenIndicator(generations[slot], 240, 325 + slot * 240, 1.0, 0.0);
        HUDDrawGenIndicator(generations[slot], (GENERATION_MEMBER_COUNT[generations[slot]] == 4 ? 760 : 800), 515 + slot * 240, 1.0, 180.0);
	}

	for (slot = 0; slot < 2; slot++) {
		HUDDrawGenIndicator(generations[slot+2], 840, 325 + slot * 240, 1.0, 0.0);
        HUDDrawGenIndicator(generations[slot+2], (GENERATION_MEMBER_COUNT[generations[slot+2]] == 4 ? 1360 : 1400), 515 + slot * 240, 1.0, 180.0);
	}
}

// per player info display

static void HUDDrawCoin(int x, int y, float scale, float rotation) {
    DrawTexturePro(
        GameAtlas,
        (Rectangle){ 1, 1, 202, 200 },
        (Rectangle){ x, y, 202 * scale, 200 * scale },
        ANCHOR_7,
        rotation,
        WHITE
    );
}

static void HUDDrawCoinNumber(int coins, int x, int y, float rotation) {
    if (coins > 9999) coins = 9999;
    if (coins < -999) coins = -999;
    char coinText[5];
    snprintf(coinText, 5, "%d", coins);
    Vector2 size = MeasureTextEx(*GetFocusFont(), coinText, 50, 1.0);
    DrawTextPro(*GetFocusFont(), coinText, (Vector2){ x, y }, ANCHOR_6(size.x, size.y, 1), rotation, 50, 1.0, WHITE);
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
        ANCHOR_7,
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

static const Vector2 SCREEN_CENTER = { SCREEN_W / 2.0f, SCREEN_H / 2.0f };

static const Vector2 widgetOffset = { 160, 428 };
static const Vector2 sideMultiplier = { 2, 1 };

// --- Fixed internal layout, relative to each widget's own circle center ---
static const Vector2 rectOffset       = { -234, 2 };   // rect CENTER offset (w=300,h=100)
static const Vector2 coinOffset       = { -114, 22 };
static const Vector2 coinNumberOffset = { -264, 3 };
static const Vector2 placeOffset      = { -284, 37 };
static const float   REF_ROTATION     = 180.0f;

static void HUDDrawRectangleRoundedRotated(Rectangle rec, float roundness, int segments, float rotation, Color color) {
    // rec.x/rec.y here should be the rectangle's CENTER, not top-left
    rlPushMatrix();
        rlTranslatef(rec.x, rec.y, 0.0f);
        rlRotatef(rotation, 0.0f, 0.0f, 1.0f);

        // Draw centered on the new local origin (0,0), so offset by -w/2, -h/2
        Rectangle localRec = { -rec.width / 2, -rec.height / 2, rec.width, rec.height };
        DrawRectangleRounded(localRec, roundness, segments, color);
    rlPopMatrix();
}

static void HUDDrawPlayerWidget(Vector2 circleCenter, const char* label, float rotation, int coins, int rank, bool isTurn) {
    float rad = DEG2RAD * (rotation - REF_ROTATION);

    Vector2 rc = Vector2Add(circleCenter, Vector2Rotate(rectOffset, rad));
    Vector2 cc = Vector2Add(circleCenter, Vector2Rotate(coinOffset, rad));
    Vector2 nc = Vector2Add(circleCenter, Vector2Rotate(coinNumberOffset, rad));
    Vector2 pc = Vector2Add(circleCenter, Vector2Rotate(placeOffset, rad));

    DrawCircle(circleCenter.x, circleCenter.y, 60, TABLE_BLEND);
    if (isTurn) DrawRing(circleCenter, 60, 70, 0, 360, 30, YELLOW);

    // rc is the rect's CENTER (matches DrawRectangleRoundedRotated's expectation)
    HUDDrawRectangleRoundedRotated((Rectangle){ rc.x, rc.y, 300, 100 }, 1.5f, 30, rotation, TABLE_BLEND);

    Vector2 textSize = MeasureTextEx(*GetFocusFont(), label, 70, 1.0);
    DrawTextPro(*GetFocusFont(), label, circleCenter, ANCHOR_5(textSize.x, textSize.y, 1), rotation, 70, 1.0, WHITE);

    HUDDrawCoin(cc.x, cc.y, 0.2f, rotation);
    HUDDrawCoinNumber(coins, nc.x, nc.y, rotation);
    HUDDrawPlace(rank, pc.x, pc.y, 0.2f, rotation);
}

void HUDDrawPlayers(Player players[4], int turnIndex) {
    int ranks[4];
    HUDCalculatePlayerRank(players, ranks);

    Vector2 p1Center = Vector2Add(SCREEN_CENTER, widgetOffset);
    Vector2 p3Center = Vector2Subtract(SCREEN_CENTER, widgetOffset);
    Vector2 p2Center = Vector2Add(SCREEN_CENTER, Vector2Multiply(Vector2Rotate(widgetOffset, DEG2RAD * 90), sideMultiplier));
    Vector2 p4Center = Vector2Add(SCREEN_CENTER, Vector2Multiply(Vector2Rotate(widgetOffset, DEG2RAD * -90), sideMultiplier));

    HUDDrawPlayerWidget(p1Center, "P1", 180.0f, players[0].coins, ranks[0], turnIndex == 0);
    HUDDrawPlayerWidget(p2Center, "P2", 270.0f, players[1].coins, ranks[1], turnIndex == 1);
    HUDDrawPlayerWidget(p3Center, "P3",   0.0f, players[2].coins, ranks[2], turnIndex == 2);
    HUDDrawPlayerWidget(p4Center, "P4",  90.0f, players[3].coins, ranks[3], turnIndex == 3);
}

// pokajan! animation

static int activePokajanAnim = -1;
static float pokajanAnimTimer = 0;
static int pokajanAnimPhase = 0;

static void HUDDrawPokajanLogo(int x, int y, float scale, float rotation) {
    DrawTexturePro(
        GameAtlas,
        (Rectangle){ 1, 203, 1355, 661 },
        (Rectangle){ x, y, 1355 * scale, 661 * scale },
        ANCHOR_5(1355, 661, scale),
        rotation,
        WHITE
    );
}

void HUDInitPokajanAnim(int playerIndex) {
    activePokajanAnim = playerIndex;
    pokajanAnimPhase = 1;
    pokajanAnimTimer = 0;
}

void HUDUpdatePokajanAnim(void) {
    if (activePokajanAnim == -1) return;
    pokajanAnimTimer += pokajanAnimPhase;
    
    if (pokajanAnimTimer == 100) {
        pokajanAnimPhase = -8;
    } else if (pokajanAnimPhase != 1 && pokajanAnimTimer <= 0) {
        pokajanAnimPhase = 0;
        pokajanAnimTimer = 0;
        activePokajanAnim = -1;
    }
}

void HUDDrawPokajanAnim(void) {
    switch (activePokajanAnim) {
        case 0:
            DrawRectangleGradientV(0, 270, 1920, 810, BLANK, (Color){ 0, 0, 0, fmin(pokajanAnimTimer * 8, 255) });
            HUDDrawPokajanLogo(960, 1170 - (350 * log(pokajanAnimTimer + 1) / LN_70), 0.5f, 180.0f);
            break;
        case 2:
            DrawRectangleGradientV(0, 0, 1920, 810, (Color){ 0, 0, 0, fmin(pokajanAnimTimer * 8, 255) }, BLANK);
            HUDDrawPokajanLogo(960, (350 * log(pokajanAnimTimer + 1) / LN_70) - 90, 0.5f, 0.0f);
            break;
        case 1:
            DrawRectangleGradientH(0, 0, 1440, 1080, (Color){ 0, 0, 0, fmin(pokajanAnimTimer * 8, 255) }, BLANK);
            HUDDrawPokajanLogo((350 * log(pokajanAnimTimer + 1) / LN_70) - 90, 540, 0.5f, 270.0f);
            break;
        case 3:
            DrawRectangleGradientH(480, 0, 1440, 1080, BLANK, (Color){ 0, 0, 0, fmin(pokajanAnimTimer * 8, 255) });
            HUDDrawPokajanLogo(2010 - (350 * log(pokajanAnimTimer + 1) / LN_70), 540, 0.5f, 90.0f);
            break;
        default:
            return;
    }
}


void HUDUnload(void) {
    UnloadTexture(GameAtlas);
}