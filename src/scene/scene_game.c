#include "scene_game.h"
#include <raylib.h>
#include <stdlib.h>
#include "scene_manager.h"
#include "overlay_card_instructions.h"
#include "overlay_pokajan_anim.h"
#include "../component/component_card.h"
#include "../component/component_hud.h"
#include "../component/component_char_portrait.h"
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
} GameScene;

#ifdef F_DEBUG
// Applies a payment the same way the engine does: payer clamps at 0, receiver gets the full amount.
static void GameFakePay(TableEvent *event, int from, int to, int amount) {
	event->coinsAfter[from] -= amount;
	if (event->coinsAfter[from] < 0) event->coinsAfter[from] = 0;
	event->coinsAfter[to] += amount;
}

// Builds a plausible EVENT_POKAJAN from the current table without touching it.
static TableEvent GameFakePokajanEvent(PokajanTable *table, int standId, bool fromDiscard) {
	Game *g = &table->game;

	TableEvent event = { 0 };
	event.type = EVENT_POKAJAN;
	event.standId = standId;
	event.fromDiscardOf = -1;
	event.card = EMPTY_CARD;
	event.borrowed = EMPTY_CARD;
	event.match = EMPTY_MATCH;
	for (int i = 0; i < 4; i++) {
		event.coinsBefore[i] = event.coinsAfter[i] = g->players[i].coins;
	}

	Match *m = &event.match;
	m->playerIndex = standId;
	m->complete = true;

	if (!fromDiscard) {
		// three of a kind, all pink, first member of the first generation
		Generation gen = g->generations[0];
		Card c = { .id = GENERATIONS[gen][0], .generation = gen, .variant = V_PINK };

		m->pattern = THREE_OF_A_KIND;
		m->colorState = SAME;
		m->useDiscardOf = -1;
		for (int i = 0; i < 3; i++) m->matchInHand[i] = c;
		m->reward = 840 + (IS_SAME_MEMBER(c, g->bonusCard) ? 3 * 90 : 0);

		for (int i = 0; i < 4; i++) {
			if (i != standId) GameFakePay(&event, i, standId, m->reward / 3);
		}
	} else {
		// full generation, colors cycling blue/pink/orange, last member taken from the discard
		Generation gen = g->generations[1];
		int count = GENERATION_MEMBER_COUNT[gen];
		int discarder = (standId + 3) % 4; // player before, as in real play
		int bonus = 0;

		m->pattern = FULL_GENERATION;
		m->colorState = DIFFERENT;
		m->useDiscardOf = discarder;

		for (int i = 0; i < count; i++) {
			Card c = { .id = GENERATIONS[gen][i], .generation = gen, .variant = (Variant)(i % 3) };
			if (IS_SAME_MEMBER(c, g->bonusCard)) bonus += 90;
			if (i < count - 1) m->matchInHand[i] = c;
			else event.borrowed = c;
		}

		int base = (count == 3) ? 180 : (count == 4) ? 300 : 480;
		m->reward = base + bonus;
		event.fromDiscardOf = discarder;

		GameFakePay(&event, discarder, standId, m->reward);
	}

	return event;
}
#endif

static void GameInit(void *self) {
	GameScene *s = (GameScene *)self;
	s->cardSpacing = 0;

	CardLoad(s->table->game.generations);
	SoundEnsureCharacterVoiceLoaded();
	CharPortraitEnsureLoaded();
}

static void GameStart(void *self) {
	GameScene *s = (GameScene *)self;
	// SceneManagerPush(CardInstructionsCreate(s->table)); // TODO: add back in
}

static void GameUpdate(void *self) {
	GameScene *s = (GameScene *)self;
	if (s->cardSpacing < GEN_MAX_WIDTH) s->cardSpacing += 20;

	/* TODO: complete
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
				// handle in overlay
				break;
			default:
				break;
		}
	}
		*/

	#ifdef F_DEBUG
		uint8_t p = GetPokajanPressed();
		const uint8_t seatBits[4] = { P1, P2, P3, P4 };
		for (int i = 0; i < 4; i++) {
			if (p & seatBits[i]) {
				TableEvent fake = GameFakePokajanEvent(s->table, i, IsKeyDown(KEY_LEFT_SHIFT));
				SceneManagerPush(PokajanAnimCreate(s->table, &fake));
				break;
			}
		}
	#endif
}

static void GameRender(void *self) {
	GameScene *s = (GameScene *)self;
	ClearBackground(DARKGREEN);

	// card on bottom
	DrawRectangleRoundedLinesEx((Rectangle){ 210, 260, 1500, 550 }, 0.2, 30, 10, TABLE_BLEND);
	int slot = 0;
	for (slot = 0; slot < 2; slot++) {
		int memCount = GENERATION_MEMBER_COUNT[s->table->game.generations[slot]];
		float spacePerMem = (float)(memCount == 4 ? s->cardSpacing - 40 : s->cardSpacing) / (memCount - 1);
		for (int mem = 0; mem < memCount; mem++) {
			CardDrawRaw(slot, mem, V_DISPLAY, 260 + spacePerMem * mem, 310 + slot * 240, 0.6);
		}
	}

	
	for (slot = 0; slot < 2; slot++) {
		int memCount = GENERATION_MEMBER_COUNT[s->table->game.generations[slot + 2]];
		float spacePerMem = (float)(memCount == 4 ? s->cardSpacing - 40 : s->cardSpacing) / (memCount - 1);
		for (int mem = 0; mem < memCount; mem++) {
			CardDrawRaw(slot + 2, mem, V_DISPLAY, 860 + spacePerMem * mem, 310 + slot * 240, 0.6);
		}
	}
		

	if (s->cardSpacing >= GEN_MAX_WIDTH) HUDDrawGenIndicators(s->table->game.generations);

	CardDraw(s->table->game.bonusCard, 1440, 380, 0.9);
	DrawFocusTextUpsideDown("BONUS", (Vector2){ 1455, 335 }, 70, TABLE_BLEND);
	DrawFocusText("BONUS", (Vector2){ 1455, 715 }, 70, TABLE_BLEND);

	HUDDrawSeats(s->table);
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