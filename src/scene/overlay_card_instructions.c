#include "overlay_card_instructions.h"
#include <raylib.h>
#include <stdlib.h>
#include <stdio.h>
#include "scene_manager.h"
#include "../component/component_card.h"
#include "../utils/input.h"
#include "../utils/text.h"

typedef struct {
	Scene base;
	Generation *generations;
} CardInstructionsOverlay;

static const char *GENERATION_NAME[] = { "Gen 0", "Gen 1", "Gen 2", "GAMERS", "Gen 3", "Gen 4", "Gen 5", "holoX", "Myth", "Promise", "Advent", "ID Gen 1", "ID Gen 2", "ID Gen 3", "ReGLOSS" };

static void CardInstructionsInit(void *self, Generation generations[4]) {
	CardInstructionsOverlay *s = (CardInstructionsOverlay *)self;
	s->generations = generations;
}

static void CardInstructionsStart(void *self) {
	(void)self;
}

static void CardInstructionsUpdate(void *self) {
	if (GetPokajanPressed()) SceneManagerPop();
}

static void CardInstructionsRender(void *self) {
	CardInstructionsOverlay *s = (CardInstructionsOverlay *)self;

	DrawRectangle(0, 0, 1920, 1080, (Color){ 0, 0, 0, 196 }); // bg
	DrawRectangleRounded((Rectangle){ 100, 200, 1720, 680 }, 0.2, 30, DARKGREEN);

	DrawFocusText("Preparing Your Deck:", (Vector2){ 170, 260 }, 70, WHITE);

	DrawMainText(
		TextFormat("1. Take out the cards of %s, %s, %s, and %s.\n(each member has 3 colors of cards each, and 3 cards per color.)",
			GENERATION_NAME[s->generations[0]], GENERATION_NAME[s->generations[1]], GENERATION_NAME[s->generations[2]], GENERATION_NAME[s->generations[3]]), 
		(Vector2){ 170, 360 }, 40, WHITE
	);

	DrawMainText("2. Shuffle the cards.", (Vector2){ 170, 490 }, 40, WHITE);

	int allGenMemSum = 0;
	for (int i = 0; i < 4; i++) {
		allGenMemSum += GENERATION_MEMBER_COUNT[s->generations[i]];
	}

	DrawMainText(
		TextFormat("3. After that, take out %d cards to make a 100-card deck.", (allGenMemSum * 3 * 3) - 100), 
		(Vector2){ 170, 555 }, 40, WHITE
	);

	DrawMainText("4. Then, deal 7 cards to each player.", (Vector2){ 170, 620 }, 40, WHITE);

	DrawFocusTextCenter("Press Pokajan! to close", 780, 50, WHITE);
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

Scene *CardInstructionsCreate(Generation generations[4]) {
    CardInstructionsOverlay *s = malloc(sizeof(CardInstructionsOverlay));
    s->base.vtable = &cardInstVTable;
    CardInstructionsInit(s, generations);
    return (Scene *)s;
}