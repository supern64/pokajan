#include "scene_game.h"
#include <raylib.h>
#include <stdlib.h>
#include "scene_manager.h"
#include "overlay_card_instructions.h"
#include "../utils/text.h"
#include "../component/component_card.h"
#include "../component/component_hud.h"
#include "../pokajan_core/cards.h"
#include "../pokajan_core/pokajan.h"
#include "../sound/sound.h"
#include "../utils/misc.h"
#include "../utils/input.h"

#define GEN_MAX_WIDTH 360

typedef struct {
	Scene base;
	Game game;

	float cardSpacing;
} GameScene;

static void GameInit(void *self) {
	GameScene *s = (GameScene *)self;
	s->cardSpacing = 0;

	HUDLoad();
	PokajanInit(&s->game);
	CardLoad(s->game.generations);
}

static void GameStart(void *self) {
	GameScene *s = (GameScene *)self;
	SoundPlayBGM();
	SceneManagerPush(CardInstructionsCreate(s->game.generations));
}

static void GameUpdate(void *self) {
	GameScene *s = (GameScene *)self;
	if (s->cardSpacing < GEN_MAX_WIDTH) s->cardSpacing += 20;
	HUDUpdatePokajanAnim();
}

static void GameRender(void *self) {
	GameScene *s = (GameScene *)self;
	ClearBackground(DARKGREEN);

	// card on bottom
	DrawRectangleRoundedLinesEx((Rectangle){ 210, 260, 1500, 550 }, 0.2, 30, 10, TABLE_BLEND);
	int slot = 0;
	for (slot = 0; slot < 2; slot++) {
		int memCount = GENERATION_MEMBER_COUNT[s->game.generations[slot]];
		float spacePerMem = (float)(memCount == 4 ? s->cardSpacing - 40 : s->cardSpacing) / (memCount - 1);
		for (int mem = 0; mem < memCount; mem++) {
			CardDrawRaw(slot, mem, V_DISPLAY, 260 + spacePerMem * mem, 310 + slot * 240, 0.6);
		}
	}

	
	for (slot = 0; slot < 2; slot++) {
		int memCount = GENERATION_MEMBER_COUNT[s->game.generations[slot + 2]];
		float spacePerMem = (float)(memCount == 4 ? s->cardSpacing - 40 : s->cardSpacing) / (memCount - 1);
		for (int mem = 0; mem < memCount; mem++) {
			CardDrawRaw(slot + 2, mem, V_DISPLAY, 860 + spacePerMem * mem, 310 + slot * 240, 0.6);
		}
	}
		

	if (s->cardSpacing >= GEN_MAX_WIDTH) HUDDrawGenIndicators(s->game.generations);

	CardDraw(s->game.bonusCard, 1440, 380, 0.9);
	DrawFocusTextUpsideDown("BONUS", (Vector2){ 1455, 335 }, 70, TABLE_BLEND);
	DrawFocusText("BONUS", (Vector2){ 1455, 715 }, 70, TABLE_BLEND);

	// player info on top
	HUDDrawPlayers(s->game.players, s->game.turnIndex);

	// then pokajan anim
	HUDDrawPokajanAnim();
}

static void GameDestroy(void *self) {
	SoundStopBGM();
	CardUnload();
	HUDUnload();
	free(self);
}

static const SceneVTable GameVTable = {
	.start = GameStart,
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