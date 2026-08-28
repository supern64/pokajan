#include <raylib.h>
#define RAYGUI_IMPLEMENTATION
#include "incl/raygui.h"

#include "component/component_card.h"

static Font MainFont;

typedef struct {
    Card hand[7];
    Card draw;
    Card discard;

    Color turnLight;
    Color handLight[7];
    Color drawLight;
    Color discardLight;
    Color pokajanLight;

    int coins;
} PlayerUnit;

// pre declare shit
static void InitPlayerUnitInfo(void);
static void DrawPlayerUnits(void);

static PlayerUnit units[4];

#define PLAYER_COUNT 4
#define ROW_HEIGHT   240
#define ROW_Y_BASE   48
static const int ledX[9] = { 144, 280, 416, 552, 688, 824, 960, 1136, 1312 };

int main() {
    InitWindow(1608, 1032, "Player Unit Simulator");
    SetTargetFPS(60);

    // load font
    MainFont = LoadFontEx("assets/fonts/TT-RoGSanSrfStdN-Bd.ttf", 128, 0, 0);
    GenTextureMipmaps(&MainFont.texture);
    SetTextureFilter(MainFont.texture, TEXTURE_FILTER_BILINEAR);
    GuiSetFont(MainFont);
    GuiSetStyle(DEFAULT, TEXT_SIZE, 16);

    InitPlayerUnitInfo();
    CardLoad((Generation[]){ GEN_0, GEN_1, GEN_2, GEN_3 });

    while (!WindowShouldClose()) {
        BeginDrawing();
            ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));

            DrawPlayerUnits();
            
            GuiStatusBar((Rectangle){ 0, 1008, 1608, 24 }, "Connected to 127.0.0.1");
        EndDrawing();
    }

    CloseWindow();
    return 0;
}

static void InitPlayerUnitInfo(void) {
    for (int i = 0; i < 4; i++) {
        units[i] = (PlayerUnit){
            .coins = 1000,
            .hand = { EMPTY_CARD, EMPTY_CARD, EMPTY_CARD, EMPTY_CARD, EMPTY_CARD, EMPTY_CARD, EMPTY_CARD },
            .discard = EMPTY_CARD,
            .draw = EMPTY_CARD,
            .discardLight = GRAY,
            .drawLight = GRAY,
            .pokajanLight = GRAY,
            .turnLight = GRAY,
            .handLight = { GRAY, GRAY, GRAY, GRAY, GRAY, GRAY, GRAY }
        };
    }
}

static void BtnPokajan(int playerIndex)
{
    // TODO: pokajan logic for units[playerIndex]
}

static void BtnSkipCycle(int playerIndex)
{
    // TODO: skip/cycle logic for units[playerIndex]
}

static void DrawPlayerUnits(void)
{
    for (int p = 0; p < PLAYER_COUNT; p++)
    {
        int rowY = ROW_Y_BASE + p * ROW_HEIGHT;
        PlayerUnit *u = &units[p];

        GuiLabel((Rectangle){ 24, rowY + 40, 48, 16 }, TextFormat("P%d", p + 1));
        GuiLabel((Rectangle){ 16, rowY + 56, 64, 24 }, TextFormat("%d", u->coins));

        GuiGroupBox((Rectangle){ 80,   rowY, 976, 216 }, "Hand");
        GuiGroupBox((Rectangle){ 1072, rowY, 160, 216 }, "Draw");
        GuiGroupBox((Rectangle){ 1248, rowY, 160, 216 }, "Discard");

        // turn indicator
        DrawRectangleRec((Rectangle){ 24, rowY + 96, 40, 40 }, u->turnLight);

        if (GuiButton((Rectangle){ 1424, rowY + 8,   136, 120 }, "Pokajan!"))   BtnPokajan(p);
        if (GuiButton((Rectangle){ 1432, rowY + 144, 120, 64  }, "Skip/Cycle")) BtnSkipCycle(p);

        // pokajan indicator strip
        DrawRectangleRec((Rectangle){ 1568, rowY + 16, 12, 104 }, u->pokajanLight);

        // draw cards
        for (int i = 0; i < 9; i++) {
            CardDrawRaw(0, 0, 0, ledX[i] - 48, rowY + 24, 0.5);
        }


        // draw LEDs (7 hand + draw + discard)
        for (int i = 0; i < 7; i++) {
            DrawRectangleRec((Rectangle){ (float)ledX[i], rowY + 8, 32, 12 }, u->handLight[i]);
        }
        DrawRectangleRec((Rectangle){ (float)ledX[7], rowY + 8, 32, 12 }, u->drawLight);
        DrawRectangleRec((Rectangle){ (float)ledX[8], rowY + 8, 32, 12 }, u->discardLight);
    }
}