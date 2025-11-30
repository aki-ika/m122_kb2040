#include <stdint.h>
#include <stdbool.h>
#include "print.h"
#include "util.h"
#include "debug.h"
#include "ps2.h"
#include "matrix.h"

#define print_matrix_row(row)  print_bin_reverse8(matrix_get_row(row))
#define print_matrix_header()  print("\nr/c 01234567\n")
#define ROW_SHIFTER ((uint8_t)1)

static void matrix_make(uint8_t code);
static void matrix_break(uint8_t code);

/*
 * Matrix Array usage:
 * 'Scan Code Set 3' is assigned into 17x8 cell matrix.
 *
 *    8bit wide
 *   +---------+
 *  0|         |
 *  :|         | 0x00-0x87
 *  ;|         |
 * 17|         |
 *   +---------+
 */
static uint8_t matrix[MATRIX_ROWS];
#define ROW(code)      (code>>3)
#define COL(code)      (code&0x07)

__attribute__ ((weak))
void matrix_init_user(void) {
}

void matrix_init(void)
{
    debug_enable = true;

    ps2_host_init();

    // initialize matrix state: all keys off
    for (uint8_t i=0; i < MATRIX_ROWS; i++) matrix[i] = 0x00;

    matrix_init_user();
    return;
}

uint8_t matrix_scan(void)
{
    // scan code reading states
    static enum {
        RESET,
        RESET_RESPONSE,
        KBD_ID0,
        KBD_ID1,
        CONFIG,
        READY,
        F0_BREAK,
    } state = RESET;

    uint8_t code;
    if ((code = ps2_host_recv())) {
        dprintf("r%02X ", code);
    }

    switch (state) {
        case RESET:
            dprint("wFF ");
            if (ps2_host_send(0xFF) == 0xFA) {
                dprint("[ack]\nRESET_RESPONSE: ");
                state = RESET_RESPONSE;
            }
            break;
        case RESET_RESPONSE:
            if (code == 0xAA) {
                dprint("[ok]\nKBD_ID: ");
                state = KBD_ID0;
            } else if (code) {
                dprint("err\nRESET: ");
                state = RESET;
            }
            break;
        // after reset receive keyboard ID(2 bytes)
        case KBD_ID0:
            if (code) {
                state = KBD_ID1;
            }
            break;
        case KBD_ID1:
            if (code) {
                dprint("\nCONFIG: ");
                state = CONFIG;
            }
            break;
        case CONFIG:
            dprint("wF8 ");
            if (ps2_host_send(0xF8) == 0xFA) {
                dprint("[ack]\nREADY\n");
                state = READY;
            }
            break;
        case READY:
            switch (code) {
                case 0x00:
                    break;
                case 0xF0:
                    state = F0_BREAK;
                    dprint(" ");
                    break;
                default:    // normal key make
                    if (code < 0x88) {
                        matrix_make(code);
                    } else {
                        dprintf("unexpected scan code at READY: %02X\n", code);
                    }
                    state = READY;
                    dprint("\n");
            }
            break;
        case F0_BREAK:    // Break code
            switch (code) {
                case 0x00:
                    break;
                default:
                    if (code < 0x88) {
                        matrix_break(code);
                    } else {
                        dprintf("unexpected scan code at F0: %02X\n", code);
                    }
                    state = READY;
                    dprint("\n");
            }
            break;
    }
    return 1;
}

inline
uint8_t matrix_get_row(uint8_t row)
{
    return matrix[row];
}

inline
static void matrix_make(uint8_t code)
{
    if (!matrix_is_on(ROW(code), COL(code))) {
        matrix[ROW(code)] |= 1<<COL(code);
    }
}

inline
static void matrix_break(uint8_t code)
{
    if (matrix_is_on(ROW(code), COL(code))) {
        matrix[ROW(code)] &= ~(1<<COL(code));
    }
}

bool matrix_is_on(uint8_t row, uint8_t col)
{
    return (matrix_get_row(row) & (1<<col));
}

void matrix_print(void)
{
    print("r/c 01234567\n");

    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        print_hex8(row);
        print(": ");
        print_bin_reverse8(matrix_get_row(row));
        print("\n");
    }
}
