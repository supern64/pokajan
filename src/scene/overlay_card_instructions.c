#include "overlay_card_instructions.h"
#include <raylib.h>
#include <stdlib.h>
#include <stdio.h>
#include "scene_manager.h"
#include "../network/bridge.h"
#include "../pokajan_core/pokajan.h"
#include "../component/component_card.h"
#include "../utils/input.h"
#include "../utils/text.h"

typedef struct {
	Scene base;
	PokajanTable* table;
} CardInstructionsOverlay;

static const char *GENERATION_NAME[] = { "Gen 0", "Gen 1", "Gen 2", "GAMERS", "Gen 3", "Gen 4", "Gen 5", "holoX", "Myth", "Promise", "Advent", "ID Gen 1", "ID Gen 2", "ID Gen 3", "ReGLOSS" };

static void CardInstructionsStart(void *self) {
	(void)self;
}

static void CardInstructionsUpdate(void *self) {
	CardInstructionsOverlay *s = (CardInstructionsOverlay *)self;
	if (s->table->allReady) SceneManagerPop();
}

static void CardInstructionsRender(void *self) {
	CardInstructionsOverlay *s = (CardInstructionsOverlay *)self;
	

	DrawRectangle(0, 0, 1920, 1080, (Color){ 0, 0, 0, 196 }); // bg
	DrawRectangleRounded((Rectangle){ 100, 200, 1720, 680 }, 0.2, 30, DARKGREEN);

	DrawFocusText("Preparing Your Deck:", (Vector2){ 170, 260 }, 70, WHITE);

	DrawMainText(
		TextFormat("1. Take out the cards of %s, %s, %s, and %s.\n(each member has 3 colors of cards each, and 3 cards per color.)",
			GENERATION_NAME[s->table->game.generations[0]], GENERATION_NAME[s->table->game.generations[1]], GENERATION_NAME[s->table->game.generations[2]], GENERATION_NAME[s->table->game.generations[3]]), 
		(Vector2){ 170, 360 }, 40, WHITE
	);

	DrawMainText("2. Shuffle the cards.", (Vector2){ 170, 490 }, 40, WHITE);

	int allGenMemSum = 0;
	for (int i = 0; i < 4; i++) {
		allGenMemSum += GENERATION_MEMBER_COUNT[s->table->game.generations[i]];
	}

	DrawMainText(
		TextFormat("3. After that, take out %d cards to make a 100-card deck.", (allGenMemSum * 3 * 3) - 100), 
		(Vector2){ 170, 555 }, 40, WHITE
	);

	DrawMainText("4. Deal 7 cards to each player and place them on the hand stands.", (Vector2){ 170, 620 }, 40, WHITE);

	char waitingText[25];
	snprintf(waitingText, 25, "Waiting for%s%s%s%s.", 
		s->table->seats[0].handReady ? "" : " P1", 
		s->table->seats[1].handReady ? "" : " P2", 
		s->table->seats[2].handReady ? "" : " P3", 
		s->table->seats[3].handReady ? "" : " P4"
	);
	DrawFocusTextCenter(waitingText, 780, 50, WHITE);
}

static void CardInstructionsDestroy(void *self) {
	free(self);
}

static const SceneVTable cardInstVTable = {
	.start = CardInstructionsStart,
	.update = CardInstructionsUpdate,
	.render = CardInstructionsRender,
	.destroy = CardInstructionsDestroy
};

Scene *CardInstructionsCreate(PokajanTable* table) {
    CardInstructionsOverlay *s = malloc(sizeof(CardInstructionsOverlay));
    s->base.vtable = &cardInstVTable;
    s->table = table;
    return (Scene *)s;
}