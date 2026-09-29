#include "overlay_pokajan_anim.h"
#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>
#include <math.h>
#include <stdlib.h>
#include "scene_manager.h"
#include "../component/component_hud.h"
#include "../component/component_char_mini_icon.h"
#include "../component/component_char_portrait.h"
#include "../network/bridge.h"
#include "../sound/sound.h"
#include "../utils/misc.h"
#include "../utils/text.h"

#include "../utils/input.h" // TODO remove

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

    int animCoins[4];
    int animDelta[4];
} PokajanAnimOverlay;

static void PokajanAnimStart(void *self) {

}

#define COIN_WIDGET_WIDTH 400
#define COIN_WIDGET_HEIGHT 220

// takes 0, 1, 2, 3 as rotation
static void PokajanAnimDrawCoinWidget(const char* playerName, MemberSlot member, int place, int coins, int delta, int deltaDir, Vector2 center, int rotation, int widgetAlpha) {
    float rt = rotation * 90.0f;
    float rtrad = DEG2RAD * rt;

    Rectangle topStrip, bottomStrip;

    switch (rotation) {
        case 0:
            topStrip = (Rectangle){ center.x - COIN_WIDGET_WIDTH / 2, center.y - COIN_WIDGET_HEIGHT / 2, COIN_WIDGET_WIDTH, 80 };
            bottomStrip = (Rectangle){ center.x - COIN_WIDGET_WIDTH / 2, center.y - COIN_WIDGET_HEIGHT / 2 + 80, COIN_WIDGET_WIDTH, COIN_WIDGET_HEIGHT - 80 };
            break;
        case 2:
            topStrip = (Rectangle){ center.x - COIN_WIDGET_WIDTH / 2, center.y + COIN_WIDGET_HEIGHT / 2 - 80, COIN_WIDGET_WIDTH, 80 };
            bottomStrip = (Rectangle){ center.x - COIN_WIDGET_WIDTH / 2, center.y - COIN_WIDGET_HEIGHT / 2 , COIN_WIDGET_WIDTH, COIN_WIDGET_HEIGHT - 80 };
            break;
        case 1:
            topStrip = (Rectangle){ center.y + COIN_WIDGET_HEIGHT / 2 - 80, center.x - COIN_WIDGET_WIDTH / 2, 80, COIN_WIDGET_WIDTH };
            bottomStrip = (Rectangle){center. y - COIN_WIDGET_HEIGHT / 2, center.x - COIN_WIDGET_WIDTH / 2, COIN_WIDGET_HEIGHT - 80, COIN_WIDGET_WIDTH };
            break;
        case 3:
            topStrip = (Rectangle){ center.y - COIN_WIDGET_HEIGHT / 2, center.x - COIN_WIDGET_WIDTH / 2, 80, COIN_WIDGET_WIDTH };
            bottomStrip = (Rectangle){ center.y - COIN_WIDGET_HEIGHT / 2 + 80, center.x - COIN_WIDGET_WIDTH / 2, COIN_WIDGET_HEIGHT - 80, COIN_WIDGET_WIDTH };
            break;
        default:
            return;
    }

    Rectangle widget = (Rectangle){ center.x, center.y, COIN_WIDGET_WIDTH, COIN_WIDGET_HEIGHT };

    // box
    BeginScissorMode(bottomStrip.x, bottomStrip.y, bottomStrip.width, bottomStrip.height);
        HUDDrawRectangleRoundedRotated(widget, 0.5f, 30, rt, (Color){ 235, 235, 235, widgetAlpha * 0.85f });
    EndScissorMode();
    BeginScissorMode(topStrip.x, topStrip.y, topStrip.width, topStrip.height);
        HUDDrawRectangleRoundedRotated(widget, 0.5f, 30, rt, (Color){ 235, 235, 235, widgetAlpha });
    EndScissorMode();

    Color outlineColor = (Color){ 235, 235, 235, widgetAlpha };
    if (deltaDir > 0) {
        outlineColor = ALPHA(POKAJAN_DARK_BLUE, widgetAlpha);
    } else if (deltaDir < 0) {
        outlineColor = ALPHA(POKAJAN_RED, widgetAlpha);
    }
    // outline
    HUDDrawRectangleRoundedLineRotated(widget, 0.5f, 30, rt, 8.0f, outlineColor);

    // miniicon
    Vector2 ic = Vector2Add(center, Vector2Rotate((Vector2){ 120, -70 }, rtrad));
    CharMiniIconDrawRaw(member.generation, member.slot, ic.x, ic.y, 1.0f, rt, widgetAlpha);

    // playername
    Vector2 tc = Vector2Add(center, Vector2Rotate((Vector2){ 40, -70 }, rtrad));
    Vector2 pts = MeasureTextEx(*GetMainFont(), playerName, 30.0f, 1.0f);
    DrawTextPro(*GetMainFont(), playerName, tc, ANCHOR_6(pts.x, pts.y, 1.0), rt, 30.0f, 1.0f, ALPHA(GRAY, widgetAlpha));

    // place
    Vector2 pc = Vector2Add(center, Vector2Rotate((Vector2){ -180, 5 }, rtrad));
    HUDDrawPlace(place, pc.x, pc.y, 0.3f, rt, widgetAlpha);

    // coin count
    Vector2 ctc = Vector2Add(center, Vector2Rotate((Vector2){ 180, 60 }, rtrad));
    const char* coinCount = TextFormat("%d", coins);
    Vector2 cts = MeasureTextEx(*GetFocusFont(), coinCount, 80.0f, 1.0f);
    DrawTextPro(*GetFocusFont(), coinCount, ctc, ANCHOR_6(cts.x, cts.y, 1.0), rt, 80.0f, 1.0f, ALPHA(GRAY, widgetAlpha));

    // coin icon
    Vector2 cc = Vector2Add(center, Vector2Rotate((Vector2){ 120 - cts.x, 35 }, rtrad));;
    HUDDrawCoin(cc.x, cc.y, 0.25f, rt, widgetAlpha);

    if (deltaDir != 0) {
        // delta
        Vector2 dtc = Vector2Add(center, Vector2Rotate((Vector2){ 60, 5 }, rtrad));
        const char* deltaCount = TextFormat((delta > 0) ? "+%d" : "%d", delta);
        Vector2 dts = MeasureTextEx(*GetFocusFont(), deltaCount, 60.0f, 1.0f);
        DrawTextPro(*GetFocusFont(), deltaCount, dtc, ANCHOR_6(dts.x, dts.y, 1.0), rt, 60.0f, 1.0f, outlineColor);
    }
    
}

static void PokajanAnimUpdate(void *self) {
    PokajanAnimOverlay* s = (PokajanAnimOverlay*)self;
    int p = s->pokajanEvent.standId;

    switch (s->phase) {
        case POPOUT: {
            // subphase 0 - pop in, subphase 1 - fade out
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
                
                rlSetBlendFactors(RL_DST_ALPHA, RL_ONE, RL_FUNC_ADD);
                BeginBlendMode(BLEND_CUSTOM);
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
                        1370, 730 + MIN(s->phaseTimer*s->phaseTimer, 150),// 880,
                        1.4f,
                        180.0f,
                        WHITE
                    );
                    DrawTexturePro(
                        s->pokajanLogo.texture,
                        (Rectangle){ 0, 0, s->pokajanLogo.texture.width, -s->pokajanLogo.texture.height },
                        RECT_SCALE(710, 780, s->pokajanLogo.texture.width, s->pokajanLogo.texture.height, 0.5*bounce + 0.55f),
                        ANCHOR_5(s->pokajanLogo.texture.width, s->pokajanLogo.texture.height, 0.5*bounce + 0.55f),
                        180.0f,
                        WHITE
                    );
                    break;
                case 2:
                    for (i = 0; i < 12; i++) {
                        DrawEllipse(i * 300 - s->waveTimer * 4, 0, WAVE_WIDTH, WAVE_HEIGHT_H, sec);
                    }
                    for (i = 0; i < 12; i++) {
                        DrawEllipse((i - 2) * 300 + s->waveTimer * 4, 0, WAVE_WIDTH, WAVE_HEIGHT_L, pri);
                    }
                    
                    CharPortraitDrawRaw(
                        p, 
                        (Rectangle){ 0, 0, 1024, 450 },
                        550, 350 - MIN(s->phaseTimer*s->phaseTimer, 150),// 880,
                        1.4f,
                        0.0f,
                        WHITE
                    );
                    DrawTexturePro(
                        s->pokajanLogo.texture,
                        (Rectangle){ 0, 0, s->pokajanLogo.texture.width, -s->pokajanLogo.texture.height },
                        RECT_SCALE(1210, 300, s->pokajanLogo.texture.width, s->pokajanLogo.texture.height, 0.5*bounce + 0.55f),
                        ANCHOR_5(s->pokajanLogo.texture.width, s->pokajanLogo.texture.height, 0.5*bounce + 0.55f),
                        0.0f,
                        WHITE
                    );
                    break;
                case 1:
                    for (i = 0; i < 7; i++) {
                        DrawEllipse(0, i * 300 - s->waveTimer * 4, WAVE_HEIGHT_H, WAVE_WIDTH, sec);
                    }
                    for (i = 0; i < 7; i++) {
                        DrawEllipse(0, (i - 2) * 300 + s->waveTimer * 4, WAVE_HEIGHT_L, WAVE_WIDTH, pri);
                    }

                    CharPortraitDrawRaw(
                        p, 
                        (Rectangle){ 0, 0, 1024, 450 },
                        350 - MIN(s->phaseTimer*s->phaseTimer, 150), 290,
                        1.2f,
                        270.0f,
                        WHITE
                    );
                    DrawTexturePro(
                        s->pokajanLogo.texture,
                        (Rectangle){ 0, 0, s->pokajanLogo.texture.width, -s->pokajanLogo.texture.height },
                        RECT_SCALE(300, 690, s->pokajanLogo.texture.width, s->pokajanLogo.texture.height, 0.5*bounce + 0.35f),
                        ANCHOR_5(s->pokajanLogo.texture.width, s->pokajanLogo.texture.height, 0.5*bounce + 0.35f),
                        270.0f,
                        WHITE
                    );
                    break;
                case 3:
                    for (i = 0; i < 7; i++) {
                        DrawEllipse(1920, i * 300 - s->waveTimer * 4, WAVE_HEIGHT_H, WAVE_WIDTH, sec);
                    }
                    for (i = 0; i < 7; i++) {
                        DrawEllipse(1920, (i - 2) * 300 + s->waveTimer * 4, WAVE_HEIGHT_L, WAVE_WIDTH, pri);
                    }
                    
                    CharPortraitDrawRaw(
                        p, 
                        (Rectangle){ 0, 0, 1024, 450 },
                        1570 + MIN(s->phaseTimer*s->phaseTimer, 150), 790,
                        1.2f,
                        90.0f,
                        WHITE
                    );
                    DrawTexturePro(
                        s->pokajanLogo.texture,
                        (Rectangle){ 0, 0, s->pokajanLogo.texture.width, -s->pokajanLogo.texture.height },
                        RECT_SCALE(1620, 390, s->pokajanLogo.texture.width, s->pokajanLogo.texture.height, 0.5*bounce + 0.35f),
                        ANCHOR_5(s->pokajanLogo.texture.width, s->pokajanLogo.texture.height, 0.5*bounce + 0.35f),
                        90.0f,
                        WHITE
                    );
                    break;
            }
            EndTextureMode();
            break;
        }
        case DISPLAY_CHANGE:
            // subphase 0 - fade and slide player widget in, subphase 1 - animate coin transfer, subphase 2 - animate place, subphase 3 - fade out
            if (GetPokajanPressed()) SceneManagerPop();
            if (GetSkipPressed()) s->waveTimer = (s->waveTimer + 1) % 4;
            /*
            if (s->subphase == 0 && s->subphaseTimer == 80) {
                s->subphase = 1;
                s->subphaseTimer = 0;
            } else if (s->subphase == 1 && s->subphaseTimer == 200) {
                s->subphase = 2;
                s->subphaseTimer = 0;
            } else if (s->subphase == 2 && s->subphaseTimer == 10) {
                SceneManagerPop();
            }
            */
            break;
    }
    s->phaseTimer += 1;
    s->subphaseTimer += 1;
}

static void PokajanAnimRender(void *self) {
    PokajanAnimOverlay* s = (PokajanAnimOverlay*)self;

    
    switch (s->phase) {
        case POPOUT: {
            int overlayAlpha = (s->subphase == 0) ? MIN(s->subphaseTimer * 12, 255) : MAX(255 - s->subphaseTimer * 24, 0);
            int bgAlpha = (s->subphase == 0) ? MIN(s->subphaseTimer * 4, 80) : MAX(80 - s->subphaseTimer * 8, 0);

            DrawRectangle(0, 0, SCREEN_W, SCREEN_H, (Color){ 0, 0, 0, bgAlpha } );
            DrawTextureRec(
               s->fgElements.texture,
               (Rectangle){ 0, 0, s->fgElements.texture.width, -s->fgElements.texture.height },
               (Vector2){ 0, 0 },
               WHITE_ALPHA(overlayAlpha)
            );
            break;
        }
        case DISPLAY_CHANGE: {
            const TableEvent* ev = &s->pokajanEvent;
            int overlayAlpha = (s->subphase == 0) ? MIN(s->subphaseTimer * 18, 255) : MAX(255 - s->subphaseTimer * 24, 0);
            int bgAlpha = (s->subphase == 0) ? MIN(s->subphaseTimer * 8, 128) : MAX(128 - s->subphaseTimer * 8, 0);
            DrawRectangle(0, 0, SCREEN_W, SCREEN_H, (Color){ 0, 0, 0, bgAlpha } );

            PokajanAnimDrawCoinWidget("Player 1", s->table->seats[2].member, 0, 1970, -120, -1, (Vector2){ 400, 400 }, s->waveTimer, overlayAlpha);
            break;
        }
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

    s->phase = DISPLAY_CHANGE;//POPOUT;
    s->subphase = 0;
    s->phaseTimer = 0;
    s->subphaseTimer = 0;
    s->waveTimer = 0;

    for (int i = 0; i < 4; i++) {
        s->animCoins[i] = 0;
        s->animDelta[i] = 0;
    }

    s->fgElements = LoadRenderTexture(SCREEN_W, SCREEN_H);
    s->pokajanLogo = LoadRenderTexture(1355, 661);
    return (Scene*)s;
}