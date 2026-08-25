#include "text.h"
#include <raylib.h>
#include "misc.h"

Font FocusFont;
Font MainFont;

void LoadFonts(void) {
    FocusFont = LoadFontEx("assets/fonts/LINESeedSans_A_XBd.ttf", 128, 0, 0);
    GenTextureMipmaps(&FocusFont.texture);
    SetTextureFilter(FocusFont.texture, TEXTURE_FILTER_BILINEAR);

    MainFont = LoadFontEx("assets/fonts/TT-RoGSanSrfStdN-Bd.ttf", 128, 0, 0);
    GenTextureMipmaps(&MainFont.texture);
    SetTextureFilter(MainFont.texture, TEXTURE_FILTER_BILINEAR);
}

void UnloadFonts(void) {
    UnloadFont(FocusFont);
    UnloadFont(MainFont);
}

void DrawFocusText(const char *text, Vector2 position, float fontSize, Color tint) {
    DrawTextEx(FocusFont, text, position, fontSize, 1.0, tint);
}

void DrawFocusTextUpsideDown(const char *text, Vector2 position, float fontSize, Color tint) {
    Vector2 size = MeasureTextEx(FocusFont, text, fontSize, 1.0);
    DrawTextPro(FocusFont, text, (Vector2){ position.x, position.y - size.y/2 }, size, 180.0, fontSize, 1.0, tint);
}

void DrawMainText(const char *text, Vector2 position, float fontSize, Color tint) {
    DrawTextEx(MainFont, text, position, fontSize, 1.0, tint);
}

void DrawMainTextCenter(const char *text, int posY, float fontSize, Color tint) {
    Vector2 size = MeasureTextEx(MainFont, text, fontSize, 1.0);
    DrawTextEx(MainFont, text, (Vector2){ SCREEN_W / 2 - (size.x / 2), posY }, fontSize, 1.0, tint);
}

Font* GetMainFont() {
    return &MainFont;
}

Font* GetFocusFont() {
    return &FocusFont;
}