#include "scene_game.h"
#include <raylib.h>
#include <stdlib.h>
#include "../utils/text.h"
#include "../component/component_card.h"
#include "../component/component_hud.h"
#include "../pokajan_core/cards.h"
#include "../sound/sound.h"

typedef struct {
	Scene base;

	Generation genArray[4];
	Card bonus;

	int cardSpacing;
} GameScene;

static void GameInit(void *self) {
	GameScene *s = (GameScene *)self;
	s->cardSpacing = 0;

	HUDLoad();
	GetRandomGenerations(s->genArray);
	s->bonus = GetRandomBonusCard(s->genArray);
	CardLoad(s->genArray);

	SoundPlayBGM();
}

static void GameUpdate(void *self) {
	GameScene *s = (GameScene *)self;
	if (s->cardSpacing != 90) s->cardSpacing += 5;
}

static void GameRender(void *self) {
	GameScene *s = (GameScene *)self;
	ClearBackground(DARKGREEN);
	int slot = 0;

	for (slot = 0; slot < 2; slot++) {
		for (int mem = 0; mem < GENERATION_MEMBER_COUNT[s->genArray[slot]]; mem++) {
			CardDrawRaw(slot, mem, V_DISPLAY, 260 + mem * s->cardSpacing, 275 + slot * 240, 0.6);
		}
	}

	for (slot = 0; slot < 2; slot++) {
		for (int mem = 0; mem < GENERATION_MEMBER_COUNT[s->genArray[slot + 2]]; mem++) {
			CardDrawRaw(slot + 2, mem, V_DISPLAY, 860 + mem * s->cardSpacing, 275 + slot * 240, 0.6);
		}
	}

	if (s->cardSpacing == 90) HUDDrawGenIndicators(s->genArray);

	CardDraw(s->bonus, 1440, 345, 0.9);
	DrawFocusTextUpsideDown("BONUS", (Vector2){ 1455, 300 }, 70, WHITE);
	DrawFocusText("BONUS", (Vector2){ 1455, 680 }, 70, WHITE);
}

static void GameDestroy(void *self) {
	CardUnload();
	HUDUnload();
	free(self);
}

static const SceneVTable GameVTable = {
	.init = GameInit,
	.update = GameUpdate,
	.render = GameRender,
	.destroy = GameDestroy
};

Scene *GameCreate(void) {
	GameScene *s = malloc(sizeof(GameScene));
    s->base.vtable = &GameVTable;
    GameInit(s);
    return (Scene *)s;
}