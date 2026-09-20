#include "scene_setup.h"
#include "scene_game.h"
#include "scene_manager.h"
#include "../component/component_char_mini_icon.h"
#include "../network/bridge.h"
#include "../utils/text.h"
#include "../sound/sound.h"
#include "../utils/misc.h"
#include "../utils/input.h"
#include <raylib.h>
#include <stdlib.h>

typedef struct {
    Scene base;
    PokajanTable* table;

    int memSlideAnimTimer[4];
    bool confirm[4];
    bool allSet;

    int startHold;
} SetupScene;

static void SetupInit(void *self) {
    SetupScene *s = (SetupScene *)self;

    for (int i = 0; i < 4; i++) {
        s->memSlideAnimTimer[i] = 0;
        s->confirm[i] = false;
    }
    s->allSet = false;
    s->startHold = 0;
}

static void SetupStart(void *self) {
    SoundSetBGMVolume(0.2f);
    SoundPlayBGM();
}

static void SetupUpdate(void *self) {
    SetupScene *s = (SetupScene *)self;
    uint8_t pokajanBtn = GetPokajanPressed();
    uint8_t skipCycleBtn = GetSkipPressed();

    bool stillNotReady = false;
    for (int playerIdx = 0; playerIdx < 4; playerIdx++) {
        if ((skipCycleBtn >> playerIdx) & 1) {
            if (s->confirm[playerIdx]) {
                s->confirm[playerIdx] = false;
                SoundUnloadCharacterVoiceSlot(playerIdx);
            } else {
                int count = PokajanGetPlayableCount();
                s->table->seats[playerIdx].member = PokajanLinearOrderToMember(((s->table->seats[playerIdx].member.order + count + 1) % count), true);
                s->memSlideAnimTimer[playerIdx] = 1;
            }
        }
        if ((pokajanBtn >> playerIdx) & 1 && !(s->confirm[playerIdx])) {
            // confirm input
            s->confirm[playerIdx] = true;
            SoundLoadCharacterVoiceIntoSlot(s->table->seats[playerIdx].member.id, playerIdx);
            SoundPlayCharacterVoiceFromSlot(playerIdx, SELECTED);
        }

        // update selection animation
        if (s->memSlideAnimTimer[playerIdx] > 0) {
            s->memSlideAnimTimer[playerIdx] += 10;
            if (s->memSlideAnimTimer[playerIdx] >= 150) {
                s->memSlideAnimTimer[playerIdx] = 0;
            }
        }

        if (!(s->confirm[playerIdx])) {
            stillNotReady = true;
        }
    }

    s->allSet = !stillNotReady;
    if (s->allSet) {
        if (GetPokajanDown() & P1) {
            s->startHold += 1;
            if (s->startHold == 240) {
                SceneManagerSwitchTo(GameCreate(s->table));
            }
        } else {
            s->startHold = 0;
        }
    }
}

static void SetupRender(void *self) {
    SetupScene* s = (SetupScene*)self;
    ClearBackground(DARKGREEN);

    if (s->allSet) {
        DrawFocusTextCenter("Ready!", 430, 140, WHITE);
        DrawMainTextCenter("P1, hold Pokajan! to start!", 590, 60, WHITE);
        if (s->startHold > 0) {
            DrawRectangleRounded((Rectangle){ 760, 690, 400, 30 }, 0.8f, 20, TABLE_BLEND);
            BeginScissorMode(760, 690, (int)((s->startHold / 240.0f) * 400.0f), 30);
                DrawRectangleRounded((Rectangle){ 760, 690, 400, 30 }, 0.8f, 20, WHITE);
            EndScissorMode();
        }
    } else {
        DrawMainTextCenter("Waiting for players...", 480, 80, WHITE);
    }
    

    Font *f = GetFocusFont();
    Font *m = GetMainFont();
    Vector2 size;

    int count = PokajanGetPlayableCount();

    if (s->table->seats[0].online) {
        if (s->confirm[0]) {
            DrawRectangleRounded((Rectangle){ 750, 900, 420, 220 }, 0.3f, 20, TABLE_BLEND);
            size = MeasureTextEx(*f, "OK!", 90, 1.0f);
            DrawTextPro(*f, "OK!", (Vector2){ 800, 990 }, ANCHOR_4(size.x, size.y, 1.0), 0.0f, 90, 1.0f, WHITE);
            CharMiniIconDrawRaw(s->table->seats[0].member.generation, s->table->seats[0].member.slot, 1050, 990, 1.0f, 0.0f);
        } else {
            DrawRectangleRounded((Rectangle){ 600, 900, 720, 220 }, 0.3f, 20, TABLE_BLEND);
            size = MeasureTextEx(*f, "P1", 90, 1.0f);
            DrawTextPro(*f, "P1", (Vector2){ 650, 990 }, ANCHOR_4(size.x, size.y, 1.0), 0.0f, 90, 1.0f, WHITE);

            BeginScissorMode(836, 926, 428, 128);
            int j = 0;
            for (int i = s->table->seats[0].member.order - 1; i <= s->table->seats[0].member.order + 2; i++) {
                int fOrder = (i + count - (s->memSlideAnimTimer[0] == 0 ? 0 : 1)) % count;
                MemberSlot mem = PokajanLinearOrderToMember(fOrder, true);
                CharMiniIconDrawRaw(mem.generation, mem.slot, 900 + 150 * j - s->memSlideAnimTimer[0], 990, 1.0f, 0.0f);
                j++;
            }
            EndScissorMode();
            DrawRing((Vector2){ 1050, 990 }, 64.0f, 74.0f, 0.0f, 360.0f, 30, YELLOW);
        }
    } else {
        size = MeasureTextEx(*m, "Waiting for P1...", 40, 1.0f);
        DrawTextPro(*m, "Waiting for P1...", (Vector2){ 960, 1000 }, ANCHOR_5(size.x, size.y, 1.0), 0.0f, 40, 1.0f, TABLE_BLEND);
    }

    if (s->table->seats[1].online) {
        if (s->confirm[1]) {
            DrawRectangleRounded((Rectangle){ -40, 330, 220, 420 }, 0.3f, 20, TABLE_BLEND);
            size = MeasureTextEx(*f, "OK!", 90, 1.0f);
            DrawTextPro(*f, "OK!", (Vector2){ 90, 380 }, ANCHOR_4(size.x, size.y, 1.0), 90.0f, 90, 1.0f, WHITE);
            CharMiniIconDrawRaw(s->table->seats[1].member.generation, s->table->seats[1].member.slot, 90, 630, 1.0f, 90.0f);
        } else {
            DrawRectangleRounded((Rectangle){ -40, 180, 220, 720 }, 0.3f, 20, TABLE_BLEND);
            size = MeasureTextEx(*f, "P2", 90, 1.0f);
            DrawTextPro(*f, "P2", (Vector2){ 90, 230 }, ANCHOR_4(size.x, size.y, 1.0), 90.0f, 90, 1.0f, WHITE);

            BeginScissorMode(26, 416, 128, 428);
            int j = 0;
            for (int i = s->table->seats[1].member.order - 1; i <= s->table->seats[1].member.order + 2; i++) {
                int fOrder = (i + count - (s->memSlideAnimTimer[1] == 0 ? 0 : 1)) % count;
                MemberSlot mem = PokajanLinearOrderToMember(fOrder, true);
                CharMiniIconDrawRaw(mem.generation, mem.slot, 90, 480 + 150 * j - s->memSlideAnimTimer[1], 1.0f, 90.0f);
                j++;
            }
            EndScissorMode();
            DrawRing((Vector2){ 90, 630 }, 64.0f, 74.0f, 0.0f, 360.0f, 30, YELLOW);
        }
    } else {
        size = MeasureTextEx(*m, "Waiting for P2...", 40, 1.0f);
        DrawTextPro(*m, "Waiting for P2...", (Vector2){ 80, 540 }, ANCHOR_5(size.x, size.y, 1.0), 90.0f, 40, 1.0f, TABLE_BLEND);
    }

    if (s->table->seats[2].online) {
        if (s->confirm[2]) {
            DrawRectangleRounded((Rectangle){ 750, -40, 420, 220 }, 0.3f, 20, TABLE_BLEND);
            size = MeasureTextEx(*f, "OK!", 90, 1.0f);
            DrawTextPro(*f, "OK!", (Vector2){ 1120, 90 }, ANCHOR_4(size.x, size.y, 1.0), 180.0f, 90, 1.0f, WHITE);
            CharMiniIconDrawRaw(s->table->seats[2].member.generation, s->table->seats[2].member.slot, 870, 90, 1.0f, 180.0f);
        } else {
            DrawRectangleRounded((Rectangle){ 600, -40, 720, 220 }, 0.3f, 20, TABLE_BLEND);
            size = MeasureTextEx(*f, "P3", 90, 1.0f);
            DrawTextPro(*f, "P3", (Vector2){ 1270, 90 }, ANCHOR_4(size.x, size.y, 1.0), 180.0f, 90, 1.0f, WHITE);

            BeginScissorMode(656, 26, 428, 128);
            int j = 0;
            for (int i = s->table->seats[2].member.order - 1; i <= s->table->seats[2].member.order + 2; i++) {
                int fOrder = (i + count - (s->memSlideAnimTimer[2] == 0 ? 0 : 1)) % count;
                MemberSlot mem = PokajanLinearOrderToMember(fOrder, true);
                CharMiniIconDrawRaw(mem.generation, mem.slot, 1020 - 150 * j + s->memSlideAnimTimer[2], 90, 1.0f, 180.0f);
                j++;
            }
            EndScissorMode();
            DrawRing((Vector2){ 870, 90 }, 64.0f, 74.0f, 0.0f, 360.0f, 30, YELLOW);
        }
    } else {
        size = MeasureTextEx(*m, "Waiting for P3...", 40, 1.0f);
        DrawTextPro(*m, "Waiting for P3...", (Vector2){ 960, 80 }, ANCHOR_5(size.x, size.y, 1.0), 180.0f, 40, 1.0f, TABLE_BLEND);
    }

    if (s->table->seats[3].online) {
        if (s->confirm[3]) {
            DrawRectangleRounded((Rectangle){ 1740, 330, 220, 420 }, 0.3f, 20, TABLE_BLEND);
            size = MeasureTextEx(*f, "OK!", 90, 1.0f);
            DrawTextPro(*f, "OK!", (Vector2){ 1830, 700 }, ANCHOR_4(size.x, size.y, 1.0), 270.0f, 90, 1.0f, WHITE);
            CharMiniIconDrawRaw(s->table->seats[3].member.generation, s->table->seats[3].member.slot, 1830, 450, 1.0f, 270.0f);
        } else {
            DrawRectangleRounded((Rectangle){ 1740, 180, 220, 720 }, 0.3f, 20, TABLE_BLEND);
            size = MeasureTextEx(*f, "P4", 90, 1.0f);
            DrawTextPro(*f, "P4", (Vector2){ 1830, 850 }, ANCHOR_4(size.x, size.y, 1.0), 270.0f, 90, 1.0f, WHITE);

            BeginScissorMode(1766, 236, 128, 428);
            int j = 0;
            for (int i = s->table->seats[3].member.order - 1; i <= s->table->seats[3].member.order + 2; i++) {
                int fOrder = (i + count - (s->memSlideAnimTimer[3] == 0 ? 0 : 1)) % count;
                MemberSlot mem = PokajanLinearOrderToMember(fOrder, true);
                CharMiniIconDrawRaw(mem.generation, mem.slot, 1830, 600 - 150 * j + s->memSlideAnimTimer[3], 1.0f, 270.0f);
                j++;
            }
            EndScissorMode();
            DrawRing((Vector2){ 1830, 450 }, 64.0f, 74.0f, 0.0f, 360.0f, 30, YELLOW);
        }
    } else {
        size = MeasureTextEx(*m, "Waiting for P4...", 40, 1.0f);
        DrawTextPro(*m, "Waiting for P4...", (Vector2){ 1840, 540 }, ANCHOR_5(size.x, size.y, 1.0), 270.0f, 40, 1.0f, TABLE_BLEND);
    }
    
}

static void SetupDestroy(void *self) {
    free(self);
}

static const SceneVTable setupVTable = {
	.start = SetupStart,
	.update = SetupUpdate,
	.render = SetupRender,
	.destroy = SetupDestroy
};

Scene *SetupCreate(PokajanTable* table) {
    SetupScene *s = malloc(sizeof(SetupScene));
    s->base.vtable = &setupVTable;
    s->table = table;
    SetupInit(s);
    return (Scene *)s;
}