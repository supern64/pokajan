#ifndef INPUT_H
#define INPUT_H
#include <stdint.h>

/*
    This is meant to abstract inputs from both a keyboard (for debugging) and the eventual ESP32s from the player console.
    Pokajan! button -> mapped to A, D, Z, C for P1-P4
    Skip/Cycle button -> mapped to S, F, X, V for P1-P4
*/

typedef enum {
    POKAJAN,
    SKIP_CYCLE
} Button;

#define P1 0b0001
#define P2 0b0010
#define P3 0b0100
#define P4 0b1000

// Check which Pokajan! buttons were just pressed, if any. Returns a 4-bit bitmask for players 1-4.
uint8_t GetPokajanPressed(void);

// Check which Pokajan! buttons are currently pressed, if any. Returns a 4-bit bitmask for players 1-4.
uint8_t GetPokajanDown(void);

// Check which Skip/Cycle buttons were just pressed, if any. Returns a 4-bit bitmask for players 1-4.
uint8_t GetSkipPressed(void);

// Check which Skip/Cycle buttons are currently pressed, if any. Returns a 4-bit bitmask for players 1-4.
uint8_t GetSkipDown(void);

// Registers an input press from the player console.
void InputPress(int player, Button button);

// Registers an input release from the player console.
void InputRelease(int player, Button button);

#endif