#include "input.h"
#include <raylib.h>

/*
    This is meant to abstract inputs from both a keyboard (for debugging) and the eventual ESP32s from the player console.
    Pokajan! button -> mapped to A, D, Z, C for P1-P4
    Skip/Cycle button -> mapped to S, F, X, V for P1-P4
*/

uint8_t GetPokajanPressed(void) {
    return IsKeyPressed(KEY_A) | IsKeyPressed(KEY_D) << 1 | IsKeyPressed(KEY_Z) << 2 | IsKeyPressed(KEY_C) << 3;
}

uint8_t GetPokajanDown(void) {
    return IsKeyDown(KEY_A) | IsKeyDown(KEY_D) << 1 | IsKeyDown(KEY_Z) << 2 | IsKeyDown(KEY_C) << 3;
}

uint8_t GetSkipPressed(void) {
    return IsKeyPressed(KEY_S) | IsKeyPressed(KEY_F) << 1 | IsKeyPressed(KEY_X) << 2 | IsKeyPressed(KEY_V) << 3;
}

uint8_t GetSkipDown(void) {
    return IsKeyDown(KEY_S) | IsKeyDown(KEY_F) << 1 | IsKeyDown(KEY_X) << 2 | IsKeyDown(KEY_V) << 3;
}
