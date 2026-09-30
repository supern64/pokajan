#include "scene_result.h"
#include <raylib.h>
#include <stdlib.h>
#include "scene_manager.h"
#include "scene_setup.h"
#include "../component/component_hud.h"
#include "../component/component_char_mini_icon.h"
#include "../component/component_char_portrait.h"
#include "../network/bridge.h"
#include "../utils/text.h"
#include "../utils/misc.h"
#include "../utils/input.h"

typedef struct {
	Scene base;

    PokajanTable* table;
    int slideTimer;
    int holdTimer;

    int playerByRank[4];
    int rankByPlayer[4];
} ResultScene;

static void ResultDrawResultWidget(MemberSlot slot, const char* text, int place, int coins, int x, int y, bool highlight) {
    HUDDrawPlace(place, x, y + 80, 0.4f, 1.0f, 255);
    DrawRectangleRounded((Rectangle){ x + 100, y, 600, 150 }, 1.0f, 12, POKAJAN_OFF_WHITE);
    if (highlight) DrawRectangleRoundedLinesEx((Rectangle){ x + 100, y, 600, 150 }, 1.0f, 12, 10.0f, YELLOW);

    CharMiniIconDrawRaw(slot.generation, slot.slot, x + 180, y + 75, 1.0f, 1.0f, 255);

    DrawMainText(text, (Vector2){ x + 260, y + 20 }, 30.0f, GRAY);
    DrawRectangle(x + 260, y + 65, 400, 3, GRAY);
    HUDDrawCoin(x + 260, y + 80, 0.27f, 0.0f, 255);
    
    const char* coinText = TextFormat("%d", coins);
    Vector2 size = MeasureTextEx(*GetFocusFont(), coinText, 65.0f, 1.0f);
    DrawTextPro(*GetFocusFont(), coinText, (Vector2){ x + 660, y + 105 }, ANCHOR_6(size.x, size.y, 1), 0.0f, 65.0f, 1.0f, GRAY);
}

static void ResultCalculatePlayerRank(const Player players[4], int outRankByPlayer[4], int outPlayerByRank[4]) {
    int order[4] = {0, 1, 2, 3};

    for (int i = 1; i < 4; i++) {
        int key = order[i];
        int keyCoins = players[key].coins;
        int j = i - 1;
        while (j >= 0 && players[order[j]].coins < keyCoins) {
            order[j + 1] = order[j];
            j--;
        }
        order[j + 1] = key;
    }

    outRankByPlayer[order[0]] = 0;
    for (int i = 1; i < 4; i++) {
        if (players[order[i]].coins == players[order[i - 1]].coins) {
            outRankByPlayer[order[i]] = outRankByPlayer[order[i - 1]];
        } else {
            outRankByPlayer[order[i]] = i;
        }
    }

    for (int i = 0; i < 4; i++) {
        outPlayerByRank[i] = order[i];
    }
}

static void ResultStart(void *self) {
    (void)self;
}

static void ResultUpdate(void *self) {
    ResultScene* s = (ResultScene*)self;
    if (1920 - s->slideTimer > 900) s->slideTimer += 45;
    if (GetPokajanDown() & P1) {
        s->holdTimer++;
        if (s->holdTimer == 240) {
            BridgeInitTable(s->table);
            SceneManagerSwitchTo(SetupCreate(s->table));
        }
    } else {
        s->holdTimer = 0;
    }
}

static void ResultRender(void *self) {
    ResultScene* s = (ResultScene*)self;
    ClearBackground(DARKGREEN);

    DrawFocusText("Results", (Vector2){ 1200, 40 }, 100.0f, WHITE);
    CharPortraitDrawRaw(s->playerByRank[0], (Rectangle){ 0, 0, 1024, 600 }, 480, 500, 2.0f, 1.0f, WHITE);

    for (int i = 0; i < 4; i++) {
        int toDraw = s->playerByRank[i];
        ResultDrawResultWidget(
            s->table->seats[toDraw].member,
            TextFormat("Player %d", toDraw+1),
            s->rankByPlayer[toDraw],
            s->table->game.players[toDraw].coins,
            MAX(900 + 70 * i, 1920 - s->slideTimer), 180 + 200 * i,
            s->rankByPlayer[toDraw] == 0
        );
    }

    if (s->holdTimer == 0) {
        DrawFocusText("P1, hold Pokajan! to play again", (Vector2){ 1050, 980 }, 50.0f, WHITE);
    } else {
        DrawRectangleRounded((Rectangle){ 1160, 990, 400, 30 }, 0.8f, 20, TABLE_BLEND);
        BeginScissorMode(1160, 990, (int)((s->holdTimer / 240.0f) * 400.0f), 30);
            DrawRectangleRounded((Rectangle){ 1160, 990, 400, 30 }, 0.8f, 20, WHITE);
        EndScissorMode();
    }
    
}

static void ResultDestroy(void *self) {
    free(self);
}

static const SceneVTable resultVTable = {
    .start = ResultStart,
    .update = ResultUpdate,
    .render = ResultRender,
    .destroy = ResultDestroy
};

Scene *ResultCreate(PokajanTable* table) {
    ResultScene *s = malloc(sizeof(ResultScene));
    s->base.vtable = &resultVTable;
    s->table = table;

    ResultCalculatePlayerRank(s->table->game.players, s->rankByPlayer, s->playerByRank);
    s->slideTimer = 0;
    s->holdTimer = 0;
    return (Scene*)s;
}