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
#include "../component/component_card.h"
#include "../network/bridge.h"
#include "../sound/sound.h"
#include "../utils/misc.h"
#include "../utils/text.h"

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
    int animPlace[4];
    float animPlaceScale[4];
    int animBoxOffset;
    int deltaSpeed[4];
    int initialPlace[4];
    bool newPlace;

    int bgAlpha;
    int overlayAlpha;
} PokajanAnimOverlay;

static void PokajanAnimStart(void *self) {
    (void)self;
}

#define COIN_WIDGET_WIDTH 400
#define COIN_WIDGET_HEIGHT 220

// takes 0, 1, 2, 3 as rotation
static void PokajanAnimDrawCoinWidget(const char* playerName, MemberSlot member, int place, int coins, int delta, int deltaDir, Vector2 center, int rotation, int widgetAlpha, float placeScale) {
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
            topStrip = (Rectangle){ center.x + COIN_WIDGET_HEIGHT / 2 - 80, center.y - COIN_WIDGET_WIDTH / 2, 80, COIN_WIDGET_WIDTH };
            bottomStrip = (Rectangle){ center.x - COIN_WIDGET_HEIGHT / 2, center.y - COIN_WIDGET_WIDTH / 2, COIN_WIDGET_HEIGHT - 80, COIN_WIDGET_WIDTH };
            break;
        case 3:
            topStrip = (Rectangle){ center.x - COIN_WIDGET_HEIGHT / 2, center.y - COIN_WIDGET_WIDTH / 2, 80, COIN_WIDGET_WIDTH };
            bottomStrip = (Rectangle){ center.x - COIN_WIDGET_HEIGHT / 2 + 80, center.y - COIN_WIDGET_WIDTH / 2, COIN_WIDGET_HEIGHT - 80, COIN_WIDGET_WIDTH };
            break;
        default:
            return;
    }

    Rectangle widget = (Rectangle){ center.x, center.y, COIN_WIDGET_WIDTH, COIN_WIDGET_HEIGHT };

    // box
    BeginScissorMode(bottomStrip.x, bottomStrip.y, bottomStrip.width, bottomStrip.height);
        HUDDrawRectangleRoundedRotated(widget, 0.5f, 12, rt, (Color){ 235, 235, 235, widgetAlpha * 0.85f });
    EndScissorMode();
    BeginScissorMode(topStrip.x, topStrip.y, topStrip.width, topStrip.height);
        HUDDrawRectangleRoundedRotated(widget, 0.5f, 12, rt, (Color){ 235, 235, 235, widgetAlpha });
    EndScissorMode();

    Color outlineColor = (Color){ 235, 235, 235, widgetAlpha };
    if (deltaDir > 0) {
        outlineColor = ALPHA(POKAJAN_DARK_BLUE, widgetAlpha);
    } else if (deltaDir < 0) {
        outlineColor = ALPHA(POKAJAN_RED, widgetAlpha);
    }
    // outline
    HUDDrawRectangleRoundedLineRotated(widget, 0.5f, 12, rt, 8.0f, outlineColor);

    // miniicon
    Vector2 ic = Vector2Add(center, Vector2Rotate((Vector2){ 120, -70 }, rtrad));
    CharMiniIconDrawRaw(member.generation, member.slot, ic.x, ic.y, 1.0f, rt, widgetAlpha);

    // playername
    Vector2 tc = Vector2Add(center, Vector2Rotate((Vector2){ 40, -70 }, rtrad));
    Vector2 pts = MeasureTextEx(*GetMainFont(), playerName, 30.0f, 1.0f);
    DrawTextPro(*GetMainFont(), playerName, tc, ANCHOR_6(pts.x, pts.y, 1.0), rt, 30.0f, 1.0f, ALPHA(GRAY, widgetAlpha));

    // place
    Vector2 pc = Vector2Add(center, Vector2Rotate((Vector2){ -120, 40 }, rtrad));
    HUDDrawPlace(place, pc.x, pc.y, 0.3f * placeScale, rt, widgetAlpha);

    // coin count
    Vector2 ctc = Vector2Add(center, Vector2Rotate((Vector2){ 180, 60 }, rtrad));
    const char* coinCount = TextFormat("%d", coins);
    Vector2 cts = MeasureTextEx(*GetFocusFont(), coinCount, 80.0f, 1.0f);
    DrawTextPro(*GetFocusFont(), coinCount, ctc, ANCHOR_6(cts.x, cts.y, 1.0), rt, 80.0f, 1.0f, ALPHA(GRAY, widgetAlpha));

    // coin icon
    Vector2 cc = Vector2Add(center, Vector2Rotate((Vector2){ 120 - cts.x, 35 }, rtrad));;
    HUDDrawCoin(cc.x, cc.y, 0.25f, rt, widgetAlpha);

    if (delta != 0) {
        // delta
        Vector2 dtc = Vector2Add(center, Vector2Rotate((Vector2){ 60, 5 }, rtrad));
        const char* deltaCount = TextFormat((delta > 0) ? "+%d" : "%d", delta);
        Vector2 dts = MeasureTextEx(*GetFocusFont(), deltaCount, 60.0f, 1.0f);
        DrawTextPro(*GetFocusFont(), deltaCount, dtc, ANCHOR_6(dts.x, dts.y, 1.0), rt, 60.0f, 1.0f, outlineColor);
    }
}

static void PokajanAnimDrawMatch(const Match* match, const Card* discard, Vector2 topLeft, int rotation, int alpha) {
    float rt = rotation * 90.0f;
    float rtrad = DEG2RAD * rt;

    // match type
    const char* matchType = (match->pattern == THREE_OF_A_KIND) ? "3-Card" : GENERATION_NAME[match->matchInHand[0].generation]; // this is safe since discards cannot make a match on its own
    Vector2 mts = MeasureTextEx(*GetMainFont(), matchType, 30.0f, 1.0f);
    DrawTextPro(*GetMainFont(), matchType, topLeft, ANCHOR_7, rt, 30.0f, 1.0f, WHITE_ALPHA(alpha));

    // rewards
    const char* reward = TextFormat("%d", match->reward);
    Vector2 ret = Vector2Add(topLeft, Vector2Rotate((Vector2){ mts.x + 20, -mts.y/2 }, rtrad));
    DrawTextPro(*GetFocusFont(), reward, ret, ANCHOR_7, rt, 55.0f, 2.0f, ALPHA(POKAJAN_DARK_BLUE, alpha));

    // matched cards
    int cards = (match->pattern) == THREE_OF_A_KIND ? 3 : GENERATION_MEMBER_COUNT[match->matchInHand[0].generation];
    for (int i = 0; i < (cards - (match->useDiscardOf != -1)); i++) {
        Vector2 cc = Vector2Add(topLeft, Vector2Rotate((Vector2){ i * 116, mts.y + 15 }, rtrad));
        CardDraw(match->matchInHand[i], cc.x, cc.y, 0.45f, rt, alpha);
    }
    if (match->useDiscardOf != -1) {
        Vector2 cc = Vector2Add(topLeft, Vector2Rotate((Vector2){ (cards - 1) * 116, mts.y + 15 }, rtrad));
        CardDraw(*discard, cc.x, cc.y, 0.45f, rt, alpha);
    }
}

static void PokajanAnimSortPlace(const int coins[4], int outRanks[4]) {
    int order[4] = {0, 1, 2, 3};

    // Sort player indices by coins descending (simple insertion sort, only 4 elements)
    for (int i = 1; i < 4; i++) {
        int key = order[i];
        int keyCoins = coins[key];
        int j = i - 1;
        while (j >= 0 && coins[order[j]] < keyCoins) {
            order[j + 1] = order[j];
            j--;
        }
        order[j + 1] = key;
    }

    // Assign ranks, handling ties (equal coins => equal rank)
    outRanks[order[0]] = 0;
    for (int i = 1; i < 4; i++) {
        if (coins[order[i]] == coins[order[i - 1]]) {
            outRanks[order[i]] = outRanks[order[i - 1]];
        } else {
            outRanks[order[i]] = i;
        }
    }
}

static void PokajanAnimUpdate(void *self) {
    PokajanAnimOverlay* s = (PokajanAnimOverlay*)self;
    int p = s->pokajanEvent.standId;

    switch (s->phase) {
        case POPOUT: {
            // subphase 0 - pop in, subphase 1 - fade out
            if (s->subphase == 0 && s->subphaseTimer == 0) { // first run
                SoundPlaySFX(SFX_DECLARE_1);
                SoundPlayCharacterVoiceFromSlot(p, GetRandomValue(0, 1) ? CV_POKAJAN_1 : CV_POKAJAN_2);
            }

            if (s->subphase == 0) {
                s->waveTimer += MAX((80 - s->subphaseTimer) / 20, 1); // should get slower as subphaseTimer increases
                s->overlayAlpha = MIN(s->subphaseTimer * 12, 255);
                s->bgAlpha = MIN(s->subphaseTimer * 4, 80);
            } else {
                s->waveTimer += 1;
                s->overlayAlpha = MAX(255 - s->subphaseTimer * 24, 0);
                s->bgAlpha = MAX(80 - s->subphaseTimer * 8, 0);
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
            // subphase 0 - fade and slide player widget in, subphase 1 - animate coin transfer, subphase 3 - animate place, subphase 4 - fade out
            switch (s->subphase) {
                case 0:
                    s->overlayAlpha = MIN(s->subphaseTimer * 18, 255);
                    s->bgAlpha = MIN(s->subphaseTimer * 8, 164);
                    s->animBoxOffset = MAX(200 - log2(s->subphaseTimer) * 34, 0);
                    if (s->subphaseTimer == 80) { // 80
                        s->subphase = 1;
                        s->subphaseTimer = 0;
                    }
                    break;
                case 1: {
                    bool allZero = true;
                    for (int i = 0; i < 4; i++) {
                        // all deltas are a multiple of 30, so ONLY use factors of 30
                        if (s->animDelta[i] != 0) {
                            s->animDelta[i] -= s->deltaSpeed[i];
                            s->animCoins[i] += s->deltaSpeed[i];

                            if ((s->animDelta[i] > 0 && s->deltaSpeed[i] < 0) || (s->animDelta[i] < 0 && s->deltaSpeed[i] > 0)) {
                                s->animDelta[i] = 0;
                                s->animCoins[i] = s->pokajanEvent.coinsAfter[i];
                            } else {
                                allZero = false;
                            }   
                        }
                    }
                    if (allZero) {
                        s->subphase = 2;
                        s->subphaseTimer = 0;
                    } else if (s->subphaseTimer % 3 == 0) {
                        SoundPlaySFX(SFX_COIN);
                    }
                    break;
                }
                case 2:
                    if (s->subphaseTimer == 20) {
                        s->subphase = 3;
                        s->subphaseTimer = 0;
                    }
                    break;
                case 3: {
                    float phase = (s->subphaseTimer / 3.0f + 1);

                    if (!s->newPlace && phase >= PI - 1) {
                        PokajanAnimSortPlace(s->pokajanEvent.coinsAfter, s->animPlace);
                        s->newPlace = true;
                    }
                    if (phase <= 2*PI) {
                        for (int i = 0; i < 4; i++) {
                            if (s->animPlace[i] == s->initialPlace[i]) continue;
                            s->animPlaceScale[i] = sinf(phase + PI) / phase + 1.0f;
                        }
                    } else {
                        for (int i = 0; i < 4; i++) {
                            s->animPlaceScale[i] = 1.0f;
                        }
                    }               
                    
                    if (s->subphaseTimer == 90) {
                        s->subphase = 4;
                        s->subphaseTimer = 0;
                    }
                    break;
                }
                case 4:
                    s->overlayAlpha = MAX(255 - s->subphaseTimer * 24, 0);
                    s->bgAlpha =  MAX(164 - s->subphaseTimer * 8, 0);
                    if (s->subphaseTimer == 10) {
                        SceneManagerPop();
                        return;
                    }
                    break;
            }
            break;
    }
    s->phaseTimer += 1;
    s->subphaseTimer += 1;
}

static void PokajanAnimRender(void *self) {
    PokajanAnimOverlay* s = (PokajanAnimOverlay*)self;
    switch (s->phase) {
        case POPOUT: {
            DrawRectangle(0, 0, SCREEN_W, SCREEN_H, (Color){ 0, 0, 0, s->bgAlpha } );
            DrawTextureRec(
               s->fgElements.texture,
               (Rectangle){ 0, 0, s->fgElements.texture.width, -s->fgElements.texture.height },
               (Vector2){ 0, 0 },
               WHITE_ALPHA(s->overlayAlpha)
            );
            break;
        }
        case DISPLAY_CHANGE: {
            DrawRectangle(0, 0, SCREEN_W, SCREEN_H, (Color){ 0, 0, 0, s->bgAlpha } );

            PokajanAnimDrawCoinWidget("Player 1", s->table->seats[0].member, s->animPlace[0], s->animCoins[0], s->animDelta[0], s->deltaSpeed[0], (Vector2){ 960, 900 + s->animBoxOffset  }, 2, s->overlayAlpha, s->animPlaceScale[0]);
            PokajanAnimDrawCoinWidget("Player 3", s->table->seats[2].member, s->animPlace[2], s->animCoins[2], s->animDelta[2], s->deltaSpeed[2], (Vector2){ 960, 180 - s->animBoxOffset  }, 0, s->overlayAlpha, s->animPlaceScale[2]);
            PokajanAnimDrawCoinWidget("Player 2", s->table->seats[1].member, s->animPlace[1], s->animCoins[1], s->animDelta[1], s->deltaSpeed[1], (Vector2){ 180 - s->animBoxOffset, 540  }, 3, s->overlayAlpha, s->animPlaceScale[1]);
            PokajanAnimDrawCoinWidget("Player 4", s->table->seats[3].member, s->animPlace[3], s->animCoins[3], s->animDelta[3], s->deltaSpeed[3], (Vector2){ 1740 + s->animBoxOffset, 540 }, 1, s->overlayAlpha, s->animPlaceScale[3]);
            
            Vector2 matchLoc;
            int rt = (s->pokajanEvent.standId + 2) % 4;
            switch (s->pokajanEvent.standId) {
                case 0:
                    matchLoc = (Vector2){ 960 + COIN_WIDGET_WIDTH / 2, 900 + s->animBoxOffset - (COIN_WIDGET_HEIGHT / 2 + 30) };
                    break;
                case 2:
                    matchLoc = (Vector2){ 960 - COIN_WIDGET_WIDTH / 2, 180 - s->animBoxOffset + (COIN_WIDGET_HEIGHT / 2 + 30) };
                    break;
                case 1:
                    matchLoc = (Vector2){ 180 - s->animBoxOffset + (COIN_WIDGET_HEIGHT / 2 + 30), 540 + COIN_WIDGET_WIDTH / 2 };
                    break;
                case 3:
                    matchLoc = (Vector2){ 1740 + s->animBoxOffset - (COIN_WIDGET_HEIGHT / 2 + 30), 540 - COIN_WIDGET_WIDTH / 2 };
                    break;
                default:
                    matchLoc = (Vector2){ 0, 0 };
                    break;
            }
            PokajanAnimDrawMatch(&s->pokajanEvent.match, &s->pokajanEvent.borrowed, matchLoc, rt, s->overlayAlpha);
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

    s->phase = POPOUT;
    s->subphase = 0;
    s->phaseTimer = 0;
    s->subphaseTimer = 0;
    s->waveTimer = 0;
    s->animBoxOffset = 0;

    PokajanAnimSortPlace(event->coinsBefore, s->initialPlace);
    for (int i = 0; i < 4; i++) {
        s->animCoins[i] = event->coinsBefore[i];
        s->animDelta[i] = event->coinsAfter[i] - event->coinsBefore[i];
        s->deltaSpeed[i] = (s->animDelta[i] == 0) ? 0 : ((s->animDelta[i] < 0) ? MIN(s->animDelta[i] / 60, -1) : MAX(s->animDelta[i] / 60, 1));
        s->animPlaceScale[i] = 1.0f;
        s->animPlace[i] = s->initialPlace[i];
    }
    s->newPlace = false;

    s->bgAlpha = 0;
    s->overlayAlpha = 0;

    s->fgElements = LoadRenderTexture(SCREEN_W, SCREEN_H);
    s->pokajanLogo = LoadRenderTexture(1355, 661);
    return (Scene*)s;
}