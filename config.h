#pragma once

// Matrix size for IBM Terminal scancode set 3
#define MATRIX_ROWS 17
#define MATRIX_COLS 8

// PS/2 pins for KB2040 (Adafruit KB2040)
// Using D1 (GP0) for DATA and D0 (GP1) for CLOCK
// IMPORTANT: Clock pin must be Data pin + 1 for PIO driver
#define PS2_DATA_PIN GP0
#define PS2_CLOCK_PIN GP1

// Command key combination
#define IS_COMMAND() ( \
    get_mods() == (MOD_BIT(KC_LSFT) | MOD_BIT(KC_RSFT) | MOD_BIT(KC_RALT) | MOD_BIT(KC_RCTL)) \
)
