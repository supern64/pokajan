#ifndef TEXT_H
#define TEXT_H
#include <raylib.h>

void LoadFonts(void);
void UnloadFonts(void);

void DrawFocusText(const char *text, Vector2 position, float fontSize, Color tint);
void DrawFocusTextUpsideDown(const char *text, Vector2 position, float fontSize, Color tint);
void DrawMainText(const char *text, Vector2 position, float fontSize, Color tint);
void DrawMainTextCenter(const char *text, int posX, float fontSize, Color tint);

#endif