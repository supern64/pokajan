#include <raylib.h>
#define RAYGUI_IMPLEMENTATION
#include "incl/raygui.h"

#include "component/component_card.h"

static Font MainFont;

int main() {
    InitWindow(1700, 500, "Player Unit Simulator");

    // load font
    MainFont = LoadFontEx("assets/fonts/TT-RoGSanSrfStdN-Bd.ttf", 128, 0, 0);
    GenTextureMipmaps(&MainFont.texture);
    SetTextureFilter(MainFont.texture, TEXTURE_FILTER_BILINEAR);
    GuiSetFont(MainFont);
    GuiSetStyle(DEFAULT, TEXT_SIZE, 16);

    SetTargetFPS(60);

    CardLoad((Generation[]){ GEN_0, GEN_1, GEN_2, GEN_3 });

    while (!WindowShouldClose()) {
        BeginDrawing();
            ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));
        EndDrawing();
    }

    CloseWindow();
    return 0;
}