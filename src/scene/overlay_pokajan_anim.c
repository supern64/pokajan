#include "overlay_pokajan_anim.h"
#include <raylib.h>
#include <rlgl.h>
#include <math.h>
#include <stdlib.h>
#include "scene_manager.h"
#include "../component/component_hud.h"
#include "../component/component_char_portrait.h"
#include "../network/bridge.h"
#include "../sound/sound.h"
#include "../utils/misc.h"

#define WAVE_WIDTH 300
#define WAVE_HEIGHT_H 200
#define WAVE_HEIGHT_L 100

typedef enum {
    POPOUT,
    DISPLAY_CHANGE
} PokajanOverlayPhase;

typedef struct {
	Scene base;
    const PokajanTable* table;
    TableEvent pokajanEvent;


    RenderTexture2D fgElements;
    RenderTexture2D pokajanLogo;

    PokajanOverlayPhase phase;
    int subphase;
    int phaseTimer;
    int subphaseTimer;
    int waveTimer;
} PokajanAnimOverlay;

static void PokajanAnimStart(void *self) {

}

static void PokajanAnimUpdate(void *self) {
    PokajanAnimOverlay* s = (PokajanAnimOverlay*)self;
    int p = s->pokajanEvent.standId;

    switch (s->phase) {
        case POPOUT: {
            if (s->subphase == 0 && s->subphaseTimer == 0) {
                SoundPlaySFX(SFX_DECLARE_1);
                SoundPlayCharacterVoiceFromSlot(p, GetRandomValue(0, 1) ? CV_POKAJAN_1 : CV_POKAJAN_2);
            }

            if (s->subphase == 0) {
                s->waveTimer += MAX((80 - s->subphaseTimer) / 20, 1); // should get slower as subphaseTimer increases
            } else {
                s->waveTimer += 1;
            }

            if (s->subphase == 0 && s->subphaseTimer == 80) {
                s->subphase = 1;
                s->subphaseTimer = 0;
            } else if (s->subphase == 1 && s->subphaseTimer == 10) {
                s->subphase = 0;
                s->subphaseTimer = 0;
                s->phaseTimer = 0;

                s->waveTimer = 0;
                s->phase = DISPLAY_CHANGE;
            }

            // since we draw onto a render texture, we ned to draw the actual elements within the update loop...
            float bounceTimer = s->subphaseTimer / 2.0f + 1;
            float bounce = (s->subphase == 0 && bounceTimer < 5*PI) ? sinf(bounceTimer) / (bounceTimer) : 0;
            const MemberSlot* slot = &s->table->seats[p].member;
            Color pri = MEMBER_COLORS[slot->generation][slot->slot][0];
            Color sec = MEMBER_COLORS[slot->generation][slot->slot][1];

            // pokajan logo is 1355x661
            BeginTextureMode(s->pokajanLogo);
                ClearBackground(BLANK);

                HUDDrawPokajanLogo(0, 0, 1.0f, 0.0f, WHITE);
                
                BeginBlendMode(BLEND_CUSTOM);
                    rlSetBlendFactors(RL_DST_ALPHA, RL_ONE, RL_FUNC_ADD);
                    DrawRectanglePro((Rectangle){ 1016 - s->phaseTimer * 15, -25, 50, 900 }, ANCHOR_7, 35.0f, (Color){ 128, 128, 128, 255 });
                    DrawRectanglePro((Rectangle){ 1086 - s->phaseTimer * 15, -25, 50, 900 }, ANCHOR_7, 35.0f, (Color){ 128, 128, 128, 255 });
                EndBlendMode();
            EndTextureMode();

            BeginTextureMode(s->fgElements);
            ClearBackground(BLANK);
            int i;
            switch (p) {
                case 0:
                    for (i = 0; i < 12; i++) {
                        DrawEllipse(i * 300 - s->waveTimer * 4, 1080, WAVE_WIDTH, WAVE_HEIGHT_H, sec);
                    }
                    for (i = 0; i < 12; i++) {
                        DrawEllipse((i - 2) * 300 + s->waveTimer * 4, 1080, WAVE_WIDTH, WAVE_HEIGHT_L, pri);
                    }

                    CharPortraitDrawRaw(
                        p, 
                        (Rectangle){ 0, 0, 1024, 450 },
                        1300, 880 + 300 * bounce,
                        1.4f,
                        180.0f,
                        WHITE
                    );
                    DrawTexturePro(
                        s->pokajanLogo.texture,
                        (Rectangle){ 0, 0, s->pokajanLogo.texture.width, -s->pokajanLogo.texture.height },
                        RECT_SCALE(660, 780, s->pokajanLogo.texture.width, s->pokajanLogo.texture.height, 0.5*bounce + 0.55f),
                        ANCHOR_5(s->pokajanLogo.texture.width, s->pokajanLogo.texture.height, 0.5*bounce + 0.55f),
                        180.0f,
                        WHITE
                    );
                    break;
                /*
                case 2:
                    for (i = 0; i < 12; i++) {
                        DrawEllipse(i * 300 - s->waveTimer * 4, offset / 1.5 - 200, WAVE_WIDTH, WAVE_HEIGHT, POKAJAN_DARK_BLUE);
                    }
                    for (i = 0; i < 12; i++) {
                        DrawEllipse((i - 2) * 300 + s->waveTimer * 4, offset / 2 - 200, WAVE_WIDTH, WAVE_HEIGHT, POKAJAN_LIGHT_BLUE);
                    }
                    
                    HUDDrawPokajanLogo(960, 90, 0.5f, 0.0f);
                    break;
                case 1:
                    for (i = 0; i < 7; i++) {
                        DrawEllipse(offset / 1.5 - 200, i * 300 - s->waveTimer * 4, WAVE_HEIGHT, WAVE_WIDTH, POKAJAN_DARK_BLUE);
                    }
                    for (i = 0; i < 7; i++) {
                        DrawEllipse(offset / 2 - 200, (i - 2) * 300 + s->waveTimer * 4, WAVE_HEIGHT, WAVE_WIDTH, POKAJAN_LIGHT_BLUE);
                    }
                    HUDDrawPokajanLogo(90, 540, 0.5f, 270.0f);
                    break;
                case 3:
                    for (i = 0; i < 7; i++) {
                        DrawEllipse(2120 - offset / 1.5, i * 300 - s->waveTimer * 4, WAVE_HEIGHT, WAVE_WIDTH, POKAJAN_DARK_BLUE);
                    }
                    for (i = 0; i < 7; i++) {
                        DrawEllipse(2120 - offset / 2, (i - 2) * 300 + s->waveTimer * 4, WAVE_HEIGHT, WAVE_WIDTH, POKAJAN_LIGHT_BLUE);
                    }
                    HUDDrawPokajanLogo(2010, 540, 0.5f, 90.0f);
                    break;
                */
            }
            EndTextureMode();
            break;
        }
        case DISPLAY_CHANGE:
            SceneManagerPop();
            break;
    }
    s->phaseTimer += 1;
    s->subphaseTimer += 1;
}

static void PokajanAnimRender(void *self) {
    PokajanAnimOverlay* s = (PokajanAnimOverlay*)self;

    switch (s->phase) {
        case POPOUT: {
            int bgAlpha = (s->subphase == 0) ? MIN(s->subphaseTimer * 4, 80) : MAX(80 - s->subphaseTimer * 8, 0);
            int overlayAlpha = (s->subphase == 0) ? MIN(s->subphaseTimer * 12, 255) : MAX(255 - s->subphaseTimer * 24, 0);

            DrawRectangle(0, 0, SCREEN_W, SCREEN_H, (Color){ 0, 0, 0, bgAlpha } );
            DrawTextureRec(
               s->fgElements.texture,
               (Rectangle){ 0, 0, s->fgElements.texture.width, -s->fgElements.texture.height },
               (Vector2){ 0, 0 },
               WHITE_ALPHA(overlayAlpha)
            );
        }
        case DISPLAY_CHANGE:
            break;
    }
}

static void PokajanAnimDestroy(void *self) {
    PokajanAnimOverlay* s = (PokajanAnimOverlay*)self;
    UnloadRenderTexture(s->fgElements);
    UnloadRenderTexture(s->pokajanLogo);
    free(self);
}

static const SceneVTable pokajanAnimVTable = {
    .start = PokajanAnimStart,
    .update = PokajanAnimUpdate,
    .render = PokajanAnimRender,
    .destroy = PokajanAnimDestroy
};

Scene *PokajanAnimCreate(const PokajanTable* table, const TableEvent* event) {
    PokajanAnimOverlay *s = malloc(sizeof(PokajanAnimOverlay));
    s->base.vtable = &pokajanAnimVTable;
    s->pokajanEvent = *event;
    s->table = table;

    s->phase = POPOUT;
    s->subphase = 0;
    s->phaseTimer = 0;
    s->subphaseTimer = 0;
    s->waveTimer = 0;

    s->fgElements = LoadRenderTexture(SCREEN_W, SCREEN_H);
    s->pokajanLogo = LoadRenderTexture(1355, 661);
    return (Scene*)s;
}