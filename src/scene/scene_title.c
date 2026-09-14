#include "scene_title.h"
#include <raylib.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "scene_manager.h"
#include "scene_setup.h"
#include "../network/bridge.h"
#include "../utils/input.h"
#include "../utils/text.h"

#define FADE_WHITE (Color){ 255, 255, 255, t->globalAlpha }
#define BLINK_WHITE (Color){ 255, 255, 255, t->blinkAlpha }

static Texture2D PokajanLogo;
static Texture2D PokajanShadeLogo;

typedef struct {
	Scene base;
	PokajanTable* table;

	int phase;
	double lastPhaseSwitch;

	int blinkAlpha;
	int blinkDirection;

	int globalAlpha;
	int fade;

	int bgLogoPosition;
	int bgLogoSpeed;
} TitleScene;

static void TitleInit(void *self) {
	TitleScene *s = (TitleScene *)self;
	s->phase = 0;
	s->lastPhaseSwitch = GetTime();
	s->blinkAlpha = 255;
	s->blinkDirection = -5;
	s->globalAlpha = 255;
	s->fade = 0;
	s->bgLogoPosition = 0;
	s->bgLogoSpeed = 1;

	PokajanLogo = LoadTexture("assets/misc/pokajan_popup.png");
	PokajanShadeLogo = LoadTexture("assets/misc/pokajan_text_hz_outline.png");
}

static void TitleStart(void *self) {
	(void)self;
}

static void TitleUpdate(void *self) {
	TitleScene *s = (TitleScene *)self;
	switch (s->fade) {
		case -1:
			s->globalAlpha -= 5;
			if (s->globalAlpha == 0) {
				s->fade = 0;
			}
			break;
		case 1:
			s->globalAlpha += 5;
			if (s->globalAlpha == 255) {
				s->fade = 0;
			}
			break;
	}

	uint8_t pokajanPressed = GetPokajanPressed();
	uint8_t skipPressed = GetSkipPressed();

	switch (s->phase) {
		case 0:
			if (s->globalAlpha == 0) {
				s->phase = 1;
				s->fade = 1;
				s->lastPhaseSwitch = GetTime();
			} else if (s->table->seats[0].online && s->fade == 0) {
				s->fade = -1;
			}
			
			break;
		case 1:
			if (s->globalAlpha == 0) {
				s->globalAlpha = 255;
				s->phase = 2;
				s->lastPhaseSwitch = GetTime();
			} else if ((pokajanPressed || GetTime() - s->lastPhaseSwitch > 6.0) && s->fade == 0) {
				s->fade = -1;
			}
			break;
		case 2:
			// blink text on title screen
			s->blinkAlpha += s->blinkDirection;
			if (s->blinkAlpha == 0 || s->blinkAlpha == 255) {
				s->blinkDirection *= -1;
			}
			s->bgLogoPosition += s->bgLogoSpeed;
			if (s->bgLogoPosition == 556) s->bgLogoPosition = 0;

			if (pokajanPressed || skipPressed) {
				s->bgLogoSpeed += 2;
			}
			if (s->bgLogoSpeed > 1) {
				s->bgLogoSpeed += 2;
				if (s->bgLogoSpeed >= 40) {
					SceneManagerSwitchTo(SetupCreate(s->table));
				}
			}
			break;
	}
}

static void TitleRender(void *self) {
	TitleScene *t = (TitleScene *)self;
	switch (t->phase) {
		case 0:
			ClearBackground(BLACK);
			DrawRectangleLines(350, 310, 1250, 450, FADE_WHITE);
			DrawMainTextCenter("Please place the stands in\nthe positions shown.\nPower on P1's stand\nand wait for it to connect.", 340, 80, FADE_WHITE);

			DrawFocusText("P1", (Vector2){ 900, 900 }, 100, FADE_WHITE);
			DrawFocusText("P3", (Vector2){ 900, 80 }, 100, FADE_WHITE);
			DrawFocusText("P2", (Vector2){ 100, 440 }, 100, FADE_WHITE);
			DrawFocusText("P4", (Vector2){ 1720, 440 }, 100, FADE_WHITE);
			break;
		case 1:
			ClearBackground(BLACK);
			DrawMainTextCenter("This is an unofficial product.", 350, 70, FADE_WHITE);
			DrawMainTextCenter("All rights to the characters represented", 450, 70, FADE_WHITE);
			DrawMainTextCenter("belong to Cover Corp.", 550, 70, FADE_WHITE);
			break;
		case 2:
			ClearBackground(DARKGREEN);
			for (int i = 0; i < 4; i++) {
				DrawTextureEx(PokajanShadeLogo, (Vector2){ t->bgLogoPosition + 556 * i, 30 }, 0.0, 1.0, (Color){ 0, 0, 0, 128 });
				DrawTextureEx(PokajanShadeLogo, (Vector2){ -t->bgLogoPosition + 556 * (i+1), 980 }, 0.0, 1.0, (Color){ 0, 0, 0, 128 });
			}
			for (int i = 1; i < 3; i++) {
				DrawTextureEx(PokajanShadeLogo, (Vector2){ t->bgLogoPosition - 556 * i, 30 }, 0.0, 1.0, (Color){ 0, 0, 0, 128 });
				DrawTextureEx(PokajanShadeLogo, (Vector2){ -t->bgLogoPosition - 556 * (i-1), 980 }, 0.0, 1.0, (Color){ 0, 0, 0, 128 });
			}
			DrawTextureEx(PokajanLogo, (Vector2){ 670 - t->bgLogoSpeed * 100, 300 }, 0.0, 0.6, WHITE);
			DrawMainTextCenter("Press any key to continue", 750, 40, BLINK_WHITE);
			break;
	}
	
}

static void TitleDestroy(void *self) {
	UnloadTexture(PokajanLogo);
	UnloadTexture(PokajanShadeLogo);
	free(self);
}

static const SceneVTable titleVTable = {
	.start = TitleStart,
	.update = TitleUpdate,
	.render = TitleRender,
	.destroy = TitleDestroy
};

Scene *TitleCreate(PokajanTable* table) {
	TitleScene *s = malloc(sizeof(TitleScene));
    s->base.vtable = &titleVTable;
	s->table = table;
    TitleInit(s);
    return (Scene *)s;
}