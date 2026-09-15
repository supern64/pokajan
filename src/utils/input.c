#include "input.h"
#include <raylib.h>

/*
    This is meant to abstract inputs from both a keyboard (for debugging) and the eventual ESP32s from the player console.
    Pokajan! button -> mapped to A, D, Z, C for P1-P4
    Skip/Cycle button -> mapped to S, F, X, V for P1-P4
*/

static uint8_t LastPokajanPress = 0;
static uint8_t PokajanDown = 0;
static uint8_t LastSkipCyclePress = 0;
static uint8_t SkipCycleDown = 0;


uint8_t GetPokajanPressed(void) {
    uint8_t final = IsKeyPressed(KEY_A) | IsKeyPressed(KEY_D) << 1 | IsKeyPressed(KEY_Z) << 2 | IsKeyPressed(KEY_C) << 3 | LastPokajanPress;
    LastPokajanPress = 0;
    return final;
}

uint8_t GetPokajanDown(void) {
    return IsKeyDown(KEY_A) | IsKeyDown(KEY_D) << 1 | IsKeyDown(KEY_Z) << 2 | IsKeyDown(KEY_C) << 3 | PokajanDown;
}

uint8_t GetSkipPressed(void) {
    uint8_t final = IsKeyPressed(KEY_S) | IsKeyPressed(KEY_F) << 1 | IsKeyPressed(KEY_X) << 2 | IsKeyPressed(KEY_V) << 3 | LastSkipCyclePress;
    LastSkipCyclePress = 0;
    return final;
}

uint8_t GetSkipDown(void) {
    return IsKeyDown(KEY_S) | IsKeyDown(KEY_F) << 1 | IsKeyDown(KEY_X) << 2 | IsKeyDown(KEY_V) << 3 | SkipCycleDown;
}

void InputPress(int player, Button button) {
    if (player < 0 || player > 3) return;
    if (button == POKAJAN) {
        LastPokajanPress |= (1 << player);
        PokajanDown |= (1 << player);
    } else {
        LastSkipCyclePress |= (1 << player);
        SkipCycleDown |= (1 << player);
    }
}

void InputRelease(int player, Button button) {
    if (player < 0 || player > 3) return;
    if (button == POKAJAN) {
        LastPokajanPress &= ~(1 << player);
        PokajanDown &= ~(1 << player);
    } else {
        LastSkipCyclePress &= ~(1 << player);
        SkipCycleDown &= ~(1 << player);
    }
}