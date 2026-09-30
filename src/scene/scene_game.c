#include "scene_game.h"
#include <raylib.h>
#include <raymath.h>
#include <stdlib.h>
#include "scene_manager.h"
#include "overlay_card_instructions.h"
#include "overlay_pokajan_anim.h"
#include "../component/component_card.h"
#include "../component/component_hud.h"
#include "../component/component_char_portrait.h"
#include "../component/component_char_mini_icon.h"
#include "../pokajan_core/cards.h"
#include "../pokajan_core/pokajan.h"
#include "../network/bridge.h"
#include "../sound/sound.h"
#include "../utils/text.h"
#include "../utils/misc.h"
#include "../utils/input.h"

#define GEN_MAX_WIDTH 360

typedef struct {
	Scene base;
	PokajanTable* table;

	float cardSpacing;
	double lastMismatch;
} GameScene;

// location of widgets around the table
static const Vector2 SCREEN_CENTER = { SCREEN_W / 2.0f, SCREEN_H / 2.0f };

static const Vector2 widgetOffset = { 160, 428 };
static const Vector2 sideMultiplier = { 2, 1 };

static const Vector2 rectOffset       = { -234, 2 };   // rect CENTER offset (w=300,h=100)
static const Vector2 coinOffset       = { -114, 22 };
static const Vector2 coinNumberOffset = { -264, 3 };
static const Vector2 placeOffset      = { -314, 5 };
static const float   REF_ROTATION     = 180.0f;

// draw player widgets
static void GameDrawPlayerTableWidget(Vector2 circleCenter, float rotation, MemberSlot member, int coins, int rank, bool isTurn) {
    float rad = DEG2RAD * (rotation - REF_ROTATION);

    Vector2 rc = Vector2Add(circleCenter, Vector2Rotate(rectOffset, rad));
    Vector2 cc = Vector2Add(circleCenter, Vector2Rotate(coinOffset, rad));
    Vector2 nc = Vector2Add(circleCenter, Vector2Rotate(coinNumberOffset, rad));
    Vector2 pc = Vector2Add(circleCenter, Vector2Rotate(placeOffset, rad));

    CharMiniIconDrawRaw(member.generation, member.slot, circleCenter.x, circleCenter.y, 1.0f, rotation, 255);
    if (isTurn) DrawRing(circleCenter, 64, 74, 0, 360, 30, YELLOW);

    // rc is the rect's CENTER (matches DrawRectangleRoundedRotated's expectation)
    HUDDrawRectangleRoundedRotated((Rectangle){ rc.x, rc.y, 300, 100 }, 1.5f, 30, rotation, TABLE_BLEND);

    HUDDrawCoin(cc.x, cc.y, 0.2f, rotation, 255);
    HUDDrawCoinNumber(coins, nc.x, nc.y, rotation, WHITE);
    HUDDrawPlace(rank, pc.x, pc.y, 0.2f, rotation, 255);
}

static void GameDrawSeats(const PokajanTable* table) {
    int ranks[4];
    HUDCalculatePlayerRank(table->game.players, ranks);

    Vector2 p1Center = Vector2Add(SCREEN_CENTER, widgetOffset);
    Vector2 p3Center = Vector2Subtract(SCREEN_CENTER, widgetOffset);
    Vector2 p2Center = Vector2Add(SCREEN_CENTER, Vector2Multiply(Vector2Rotate(widgetOffset, DEG2RAD * 90), sideMultiplier));
    Vector2 p4Center = Vector2Add(SCREEN_CENTER, Vector2Multiply(Vector2Rotate(widgetOffset, DEG2RAD * -90), sideMultiplier));

    GameDrawPlayerTableWidget(p1Center, 180.0f, table->seats[0].member, table->game.players[0].coins, ranks[0], table->game.turnIndex == 0);
    GameDrawPlayerTableWidget(p2Center, 270.0f, table->seats[1].member, table->game.players[1].coins, ranks[1], table->game.turnIndex == 1);
    GameDrawPlayerTableWidget(p3Center,   0.0f, table->seats[2].member, table->game.players[2].coins, ranks[2], table->game.turnIndex == 2);
    GameDrawPlayerTableWidget(p4Center,  90.0f, table->seats[3].member, table->game.players[3].coins, ranks[3], table->game.turnIndex == 3);
}

static void GameDrawMismatch(const PokajanTable* table) {
	DrawRectangle(0, 0, SCREEN_W, SCREEN_H, (Color){ 0, 0, 0, 230 });

	const char* text = "Please check the cards lit red on your stand.";
    Vector2 textSize = MeasureTextEx(*GetFocusFont(), text, 50.0f, 1.0f);
	for (int i = 0; i < 4; i++) {
		if (!table->seats[i].mismatch) continue;
		Vector2 coords = (Vector2){ 0, 0 };
		switch (i) {
			case 0:
				coords = (Vector2){ 960, 980 };
				break;
			case 2:
				coords = (Vector2){ 960, 100 };
				break;
			case 1:
				coords = (Vector2){ 100, 540 };
				break;
			case 3:
				coords = (Vector2){ 1820, 540 };
				break;

		}
    	DrawTextPro(*GetFocusFont(), text, coords, ANCHOR_5(textSize.x, textSize.y, 1.0), i * 90.0f, 50.0f, 1.0f, WHITE);
	}
}

static void GameInit(void *self) {
	GameScene *s = (GameScene *)self;
	s->cardSpacing = 0;
	s->lastMismatch = -1.0;

	CardLoad(s->table->game.generations);
	SoundEnsureCharacterVoiceLoaded();
	CharPortraitEnsureLoaded();
}

static void GameStart(void *self) {
	GameScene *s = (GameScene *)self;
	SceneManagerPush(CardInstructionsCreate(s->table));
}

static void GameUpdate(void *self) {
	GameScene *s = (GameScene *)self;
	if (s->cardSpacing < GEN_MAX_WIDTH) s->cardSpacing += 20;

	TableEvent event;
	while (BridgePollEvent(s->table, &event)) {
		switch (event.type) {
			case EVENT_DRAW:
				SoundPlaySFX(SFX_CARD_DRAWN);
				break;
			case EVENT_DISCARD:
				SoundPlaySFX(SFX_CARD_DISCARDED);
				break;
			case EVENT_CONTEST_OPEN:
				SoundPlaySFX(SFX_DISCARD_AVAILABLE);
				break;
			case EVENT_POKAJAN:
				SceneManagerPush(PokajanAnimCreate(s->table, &event));
				break;
			default:
				break;
		}
	}

	
	if (s->table->anyMismatch && s->table->state != WAIT_INITIAL_HANDS && s->lastMismatch == -1) {
		s->lastMismatch = GetTime();
	} else if (!s->table->anyMismatch) {
		s->lastMismatch = -1;
	}
}

static void GameRender(void *self) {
	GameScene *s = (GameScene *)self;
	ClearBackground(DARKGREEN);

	// card on bottom
	DrawRectangleRoundedLinesEx((Rectangle){ 210, 260, 1500, 550 }, 0.2, 30, 10, TABLE_BLEND);

	for (int slot = 0; slot < 2; slot++) {
		int memCount = GENERATION_MEMBER_COUNT[s->table->game.generations[slot]];
		float spacePerMem = (float)(memCount == 4 ? s->cardSpacing - 40 : s->cardSpacing) / (memCount - 1);
		for (int mem = 0; mem < memCount; mem++) {
			CardDrawRaw(slot, mem, V_DISPLAY, 260 + spacePerMem * mem, 310 + slot * 240, 0.6f, 0.0f, 255);
		}
	}
	for (int slot = 0; slot < 2; slot++) {
		int memCount = GENERATION_MEMBER_COUNT[s->table->game.generations[slot + 2]];
		float spacePerMem = (float)(memCount == 4 ? s->cardSpacing - 40 : s->cardSpacing) / (memCount - 1);
		for (int mem = 0; mem < memCount; mem++) {
			CardDrawRaw(slot + 2, mem, V_DISPLAY, 860 + spacePerMem * mem, 310 + slot * 240, 0.6f, 0.0f, 255);
		}
	}
		
	// gen indicator
	if (s->cardSpacing >= GEN_MAX_WIDTH) {
		for (int slot = 0; slot < 2; slot++) {
			HUDDrawGenIndicator(s->table->game.generations[slot], 240, 325 + slot * 240, 1.0f, 0.0f);
			HUDDrawGenIndicator(s->table->game.generations[slot], (GENERATION_MEMBER_COUNT[s->table->game.generations[slot]] == 4 ? 760 : 800), 515 + slot * 240, 1.0f, 180.0f);
		}
		for (int slot = 0; slot < 2; slot++) {
			HUDDrawGenIndicator(s->table->game.generations[slot+2], 840, 325 + slot * 240, 1.0f, 0.0f);
			HUDDrawGenIndicator(s->table->game.generations[slot+2], (GENERATION_MEMBER_COUNT[s->table->game.generations[slot+2]] == 4 ? 1360 : 1400), 515 + slot * 240, 1.0f, 180.0f);
		}
	}

	// bonus card
	CardDraw(s->table->game.bonusCard, 1440, 380, 0.9, 0.0f, 255);
	DrawFocusTextUpsideDown("BONUS", (Vector2){ 1455, 335 }, 70, TABLE_BLEND);
	DrawFocusText("BONUS", (Vector2){ 1455, 715 }, 70, TABLE_BLEND);

	// seat widgets
	GameDrawSeats(s->table);

	if (s->lastMismatch != -1) { // any mismatch
		bool isAnyUnexplained = false;
		for (int p = 0; p < 4; p++) {
			if (!s->table->seats[p].mismatch) continue;
			for (int sl = 0; sl < 9; sl++) {
				if (s->table->seats[p].slotState[sl] == SLOT_UNEXPECTED) {
					isAnyUnexplained = true;
					break;
				}
			}
			if (isAnyUnexplained) break;
		}

		if (isAnyUnexplained || (GetTime() - s->lastMismatch > 5.0)) {
			GameDrawMismatch(s->table);
		}
	}
}

static void GameDestroy(void *self) {
	CardUnload();
	free(self);
}

static const SceneVTable GameVTable = {
	.start = GameStart,
	.update = GameUpdate,
	.render = GameRender,
	.destroy = GameDestroy
};

Scene *GameCreate(PokajanTable *table) {
	GameScene *s = malloc(sizeof(GameScene));
    s->base.vtable = &GameVTable;
	s->table = table;
    GameInit(s);
    return (Scene *)s;
}