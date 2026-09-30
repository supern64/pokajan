#include "overlay_game_end.h"
#include <raylib.h>
#include <stdlib.h>
#include <stdint.h>
#include "scene_manager.h"
#include "scene_result.h"
#include "../component/component_hud.h"
#include "../sound/sound.h"
#include "../utils/misc.h"
#include "../utils/text.h"

typedef struct {
	Scene base;
	PokajanTable* table;

    int fgAlpha;
    int playerAlpha[4];
    int bgAlpha;
    int phase;
    int timer;
    int rank[4];
    uint8_t zeroMask;
} GameEndOverlay;

static void GameEndStart(void *self) {
	(void)self;
}

static void GameEndUpdate(void *self) {
	GameEndOverlay *s = (GameEndOverlay *)self;

    s->timer++;
    switch (s->phase) {
        case 0:
            // fade widgets in
            if (s->zeroMask == 0 && s->timer == 1) SoundPlaySFX(SFX_OUT_OF_CARDS);
            s->bgAlpha = MIN(s->timer * 8, 164);
            s->fgAlpha = MIN(s->timer * 18, 255);
            for (int i = 0; i < 4; i++) s->playerAlpha[i] = s->fgAlpha;

            if (s->timer == 120) {
                s->timer = 0;
                s->phase = (s->zeroMask == 0) ? 2 : 1;
            }
            break;
        case 1:
            // fade out dead player (if one exists)
            if (s->timer == 1) SoundPlaySFX(SFX_OUT_OF_COINS);
            for (int i = 0; i < 4; i++) {
                if (s->zeroMask >> (i + 1) & 1) s->playerAlpha[i] = MAX(255 - s->timer * 8, 164);
            }

            if (s->timer == 120) {
                s->timer = 0;
                s->phase = 2;
            }
            break;
        case 2:
            // highlight winner
            if (s->timer == 1) SoundPlaySFX(SFX_HIGHLIGHT_WINNER);
            
            if (s->timer == 120) {
                s->timer = 0;
                s->phase = 3;
            }
            break;
        case 3:
            // fade out, force pop and switch to results screen
            s->fgAlpha = MAX(255 - s->timer * 24, 0);
            s->bgAlpha =  MAX(164 - s->timer * 8, 0);
            for (int i = 0; i < 4; i++) {
                s->playerAlpha[i] = MIN(s->playerAlpha[i], s->fgAlpha);
            }

            
            if (s->timer == 10) {
                SceneManagerPop();
                SceneManagerSwitchTo(ResultCreate(s->table));
                return;
            }
            break;
    }
}

static void GameEndRender(void *self) {
	GameEndOverlay *s = (GameEndOverlay *)self;
	DrawRectangle(0, 0, SCREEN_W, SCREEN_H, (Color){ 0, 0, 0, s->bgAlpha });

    static const Vector2 widgetCoords[] = {{ 960, 900 }, { 180, 540 }, { 960, 180 }, { 1740, 540 }};

    for (int i = 0; i < 4; i++) {
        if (s->phase >= 2 && s->rank[i] == 0) {
            Color rainbow = ColorFromHSV(s->timer * 6, 1.0f, 1.0f);
            HUDDrawCoinWidgetOutline(widgetCoords[i], ALPHA(rainbow, s->playerAlpha[i]), (i + 2) % 4);
        } else {
            HUDDrawCoinWidgetOutline(widgetCoords[i], OFF_WHITE_ALPHA(s->playerAlpha[i]), (i + 2) % 4);
        }
        HUDDrawCoinWidget(TextFormat("Player %d", i + 1), s->table->seats[i].member, s->rank[i], s->table->game.players[i].coins, widgetCoords[i], (i + 2) % 4, s->playerAlpha[i], 1.0f);
    }

    DrawMainTextCenter((s->zeroMask == 0) ? "Deck is out of cards. Ending the game." : "Player has no coins left. Ending the game.", 540, 40.0f, WHITE_ALPHA(s->fgAlpha));
}

static void GameEndDestroy(void *self) {
	free(self);
}

static const SceneVTable gameEndVTable = {
	.start = GameEndStart,
	.update = GameEndUpdate,
	.render = GameEndRender,
	.destroy = GameEndDestroy
};

Scene *GameEndCreate(PokajanTable* table) {
    GameEndOverlay *s = malloc(sizeof(GameEndOverlay));
    s->base.vtable = &gameEndVTable;
    s->table = table;

    s->fgAlpha = 0;
    s->bgAlpha = 0;
    s->phase = 0;
    s->timer = 0;

    HUDCalculatePlayerRank(table->game.players, s->rank);
    for (int i = 0; i < 4; i++) {
        if (table->game.players[i].coins == 0) {
            s->zeroMask |= 1u << i;
        }
    }

    return (Scene *)s;
}