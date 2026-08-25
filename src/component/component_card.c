#include "component_card.h"
#include <raylib.h>
#include <stdio.h>
#include "../utils/misc.h"

static Texture2D CardAtlas[4];
static Generation SlotMapping[15] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };

static void CardLoadGeneration(Generation g, int slot) {
    switch (g) {
        case GEN_0:
            CardAtlas[slot] = LoadTexture("assets/card_atlas/gen0.png");
            break;
        case GEN_1:
            CardAtlas[slot] = LoadTexture("assets/card_atlas/gen1.png");
            break;
        case GEN_2:
            CardAtlas[slot] = LoadTexture("assets/card_atlas/gen2.png");
            break;
        case GAMERS:
            CardAtlas[slot] = LoadTexture("assets/card_atlas/gamers.png");
            break;
        case GEN_3:
            CardAtlas[slot] = LoadTexture("assets/card_atlas/gen3.png");
            break;
        case GEN_4:
            CardAtlas[slot] = LoadTexture("assets/card_atlas/gen4.png");
            break;
        case GEN_5:
            CardAtlas[slot] = LoadTexture("assets/card_atlas/gen5.png");
            break;
        case HOLOX:
            CardAtlas[slot] = LoadTexture("assets/card_atlas/holox.png");
            break;
        case MYTH:
            CardAtlas[slot] = LoadTexture("assets/card_atlas/myth.png");
            break;
        case PROMISE:
            CardAtlas[slot] = LoadTexture("assets/card_atlas/promise.png");
            break;
        case ADVENT:
            CardAtlas[slot] = LoadTexture("assets/card_atlas/advent.png");
            break;
        case ID_GEN_1:
            CardAtlas[slot] = LoadTexture("assets/card_atlas/id1.png");
            break;
        case ID_GEN_2:
            CardAtlas[slot] = LoadTexture("assets/card_atlas/id2.png");
            break;
        case ID_GEN_3:
            CardAtlas[slot] = LoadTexture("assets/card_atlas/id3.png");
            break;
        case REGLOSS:
            CardAtlas[slot] = LoadTexture("assets/card_atlas/regloss.png");
            break;
    }
    GenTextureMipmaps(&CardAtlas[slot]);
    SetTextureFilter(CardAtlas[slot], TEXTURE_FILTER_BILINEAR);
}

void CardLoad(Generation gen_array[4]) {
    for (int i = 0; i < 4; i++) {
        CardLoadGeneration(gen_array[i], i);
        SlotMapping[gen_array[i]] = i;
    }
}

void CardUnload(void) {
    for (int i = 0; i < 4; i++) {
        UnloadTexture(CardAtlas[i]);
        SlotMapping[i] = -1;
    }
}

void CardDrawRaw(int slot, int memIndex, Variant variant, int x, int y, float scale) {
    // all cards are 256x363
    int texX = variant * 256;
    int texY = memIndex * 363;
    Rectangle atlasLocation = { texX, texY, 256, 363 };
    Rectangle finalLocation = { x, y, 256 * scale, 363 * scale };
    DrawTexturePro(CardAtlas[slot], atlasLocation, finalLocation, ANCHOR_7, 0.0, WHITE);
}

void CardDraw(Card card, int x, int y, float scale) {
    if (SlotMapping[card.generation] == -1) {
        printf("Attempted to render unloaded generation %d", card.generation);
        return;
    }
    int memIndex = 0;
    while (card.id != GENERATIONS[card.generation][memIndex]) {
        memIndex++;
        if (memIndex >= GENERATION_MEMBER_COUNT[card.generation]) {
            printf("Attempted to render invalid member %d of gen %d", card.id, card.generation);
            return;
        }
    }
    CardDrawRaw(SlotMapping[card.generation], memIndex, card.variant, x, y, scale);
}