// MIT License
// 
// Copyright © 2023 Greg Lund
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "boards.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "keyboard.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>


#define P0_PIN_MSK(n)   (((n) >> 5) == 0 ? (1 << ((n) & 0x1F)) : 0)
#define P1_PIN_MSK(n)   (((n) >> 5) == 1 ? (1 << ((n) & 0x1F)) : 0)

#define P0_PA_MSK  (0 \
    | P0_PIN_MSK(PA0) \
    | P0_PIN_MSK(PA1) \
    | P0_PIN_MSK(PA2) \
    | P0_PIN_MSK(PA3) \
    | P0_PIN_MSK(PA4) \
    | P0_PIN_MSK(PA5) \
    | P0_PIN_MSK(PA6) \
    | P0_PIN_MSK(PA7) \
    )

#define P1_PA_MSK  (0 \
    | P1_PIN_MSK(PA0) \
    | P1_PIN_MSK(PA1) \
    | P1_PIN_MSK(PA2) \
    | P1_PIN_MSK(PA3) \
    | P1_PIN_MSK(PA4) \
    | P1_PIN_MSK(PA5) \
    | P1_PIN_MSK(PA6) \
    | P1_PIN_MSK(PA7) \
    )

#define P0_PB_MSK  (0 \
    | P0_PIN_MSK(PB0) \
    | P0_PIN_MSK(PB1) \
    | P0_PIN_MSK(PB2) \
    | P0_PIN_MSK(PB3) \
    | P0_PIN_MSK(PB4) \
    | P0_PIN_MSK(PB5) \
    | P0_PIN_MSK(PB6) \
    | P0_PIN_MSK(PB7) \
    )

#define P1_PB_MSK  (0 \
    | P1_PIN_MSK(PB0) \
    | P1_PIN_MSK(PB1) \
    | P1_PIN_MSK(PB2) \
    | P1_PIN_MSK(PB3) \
    | P1_PIN_MSK(PB4) \
    | P1_PIN_MSK(PB5) \
    | P1_PIN_MSK(PB6) \
    | P1_PIN_MSK(PB7) \
    )

#define GET_FLAG(i, p, f) \
    (((ctx->scan_results_p0[i] ^ UINT32_MAX) & P0_PIN_MSK(p) ? f : 0) | \
     ((ctx->scan_results_p1[i] ^ UINT32_MAX) & P1_PIN_MSK(p) ? f : 0))


static const uint8_t key_table[] =
{
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,  // CRSR DOWN, F5, F3, F1, F7, CRSR RIGHT, RETURN, INST DEL
    0xff, 0x05, 0x13, 0x1a, 0x34, 0x01, 0x17, 0x33,  // LEFT SHIFT, "E", "S", "Z", "4", "A", "W", "3"
    0x18, 0x14, 0x06, 0x03, 0x36, 0x04, 0x12, 0x35,  // "X", "T", "F", "C", "6", "D", "R", "5"
    0x16, 0x15, 0x08, 0x02, 0x38, 0x07, 0x19, 0x37,  // "V", "U", "H", "B", "8", "G", "Y", "7"
    0x0e, 0x0f, 0x0b, 0x0d, 0x30, 0x0a, 0x09, 0x39,  // "N", "O" (Oscar), "K", "M", "0" (Zero), "J", "I", "9"
    0x2c, 0x00, 0x3a, 0x2e, 0x2d, 0x0c, 0x10, 0x2b,  // ",", "@", ":", ".", "-", "L", "P", "+"
    0x2f, 0x1e, 0x3d, 0xff, 0xff, 0x3b, 0x2a, 0x1c,  // "/", "^", "=", RIGHT SHIFT, HOME, ";", "*", "£"
    0xff, 0x11, 0xff, 0x20, 0x32, 0xff, 0x1f, 0x31,  // RUN STOP, "Q", "C=" (CMD), " " (SPC), "2", "CTRL", "<-", "1"
};

static const uint32_t porta_pins[] = 
{
    PA0,
    PA1,
    PA2,
    PA3,
    PA4,
    PA5,
    PA6,
    PA7,
};

static const uint32_t portb_pins[] = 
{
    PB0,
    PB1,
    PB2,
    PB3,
    PB4,
    PB5,
    PB6,
    PB7,
};


static enum scan_return key_in_row(struct keyboard_ctx* ctx, int index, uint32_t scan_result_p0, uint32_t scan_result_p1);


void keyboard_init(struct keyboard_ctx* ctx)
{
    // Set port direction
    nrf_gpio_port_dir_output_set(NRF_P0, P0_PA_MSK);
    nrf_gpio_port_dir_output_set(NRF_P1, P1_PA_MSK);

    for (int i = 0; i < sizeof(portb_pins) / sizeof(portb_pins[0]); i++)
        nrf_gpio_cfg_input(portb_pins[i], NRF_GPIO_PIN_PULLUP);

    memset(ctx->buffer_old, 0xFF, sizeof(ctx->buffer_old));
    memset(ctx->buffer, 0xFF, sizeof(ctx->buffer));
    ctx->buffer_quantity = -1;
}

struct keyboard_return keyboard_scan(struct keyboard_ctx* ctx)
{
    struct keyboard_return keyboard_return = {0};

    // Connect all Keyboard rows
    nrf_gpio_port_out_clear(NRF_P0, P0_PA_MSK);
    nrf_gpio_port_out_clear(NRF_P1, P1_PA_MSK);
    nrf_delay_us(100);

    // Check for port activity
    if (((nrf_gpio_port_in_read(NRF_P0) & P0_PB_MSK) == P0_PB_MSK) &&
        ((nrf_gpio_port_in_read(NRF_P1) & P1_PB_MSK) == P1_PB_MSK))
    {
        ctx->simultaneous_alphanumeric_keys_flag = false;
        memset(ctx->buffer_old, 0xFF, sizeof(ctx->buffer_old));
        keyboard_return.scan_return = SCAN_RETURN_NO_ACTIVITY;
        return keyboard_return;
    }

    // Wait for  all keys to be released before accepting new input
    if (ctx->simultaneous_alphanumeric_keys_flag)
    {
        keyboard_return.scan_return = SCAN_RETURN_AWAITING_NO_ACTIVITY;
        return keyboard_return;
    }

    // Scan keyboard matrix
    nrf_gpio_port_out_set(NRF_P0, P0_PA_MSK & ~(P0_PIN_MSK(porta_pins[0])));
    nrf_gpio_port_out_set(NRF_P1, P1_PA_MSK & ~(P1_PIN_MSK(porta_pins[0])));
    nrf_delay_us(100);
    ctx->scan_results_p0[7] = nrf_gpio_port_in_read(NRF_P0) & P0_PB_MSK;
    ctx->scan_results_p1[7] = nrf_gpio_port_in_read(NRF_P1) & P1_PB_MSK;

    for (int i = 6; i > -1; i--)
    {
        nrf_gpio_port_out_set(NRF_P0, P0_PIN_MSK(porta_pins[6 - i]));
        nrf_gpio_port_out_set(NRF_P1, P1_PIN_MSK(porta_pins[6 - i]));
        nrf_gpio_port_out_clear(NRF_P0, P0_PIN_MSK(porta_pins[6 - i + 1]));
        nrf_gpio_port_out_clear(NRF_P1, P1_PIN_MSK(porta_pins[6 - i + 1]));
        nrf_delay_us(100);
        ctx->scan_results_p0[i] = nrf_gpio_port_in_read(NRF_P0) & P0_PB_MSK;
        ctx->scan_results_p1[i] = nrf_gpio_port_in_read(NRF_P1) & P1_PB_MSK;
    }

    // Initialize buffer, flags and max keys
    memset(ctx->buffer_new, 0xFF, sizeof(ctx->buffer_new));
    ctx->non_alpha_flag_y = 0;

    ctx->key_quantity = MAX_KEY_ROLLOVER;
    ctx->simultaneous_keys = 0xFE;

    // Check and flag non-alphanumeric keys
    uint8_t left_shift = GET_FLAG(6, PB7, 0x40);

    uint8_t runstop = GET_FLAG(0, PB7, 0x80);
    uint8_t commodore = GET_FLAG(0, PB5, 0x20);
    uint8_t ctrl = GET_FLAG(0, PB2, 0x04);

    uint8_t right_shift = GET_FLAG(1, PB4, 0x10);
    uint8_t clr_home = GET_FLAG(1, PB3, 0x08);

    ctx->non_alpha_flag_y = left_shift | runstop | commodore | ctrl | right_shift | clr_home;

    uint8_t crsr_down = GET_FLAG(7, PB7, 0x80);
    uint8_t f5 = GET_FLAG(7, PB6, 0x40);
    uint8_t f3 = GET_FLAG(7, PB5, 0x20);
    uint8_t f1 = GET_FLAG(7, PB4, 0x10);
    uint8_t f7 = GET_FLAG(7, PB3, 0x08);
    uint8_t crsr_right = GET_FLAG(7, PB2, 0x04);
    uint8_t retkey = GET_FLAG(7, PB1, 0x02);
    uint8_t del = GET_FLAG(7, PB0, 0x01);

    ctx->non_alpha_flag_x = crsr_down | f5 | f3 | f1 | f7 | crsr_right | retkey | del;
    
    // Check for pressed keys
    for (int i = 7; i > -1; i--)
    {
        if (((ctx->scan_results_p0[i] & P0_PB_MSK) != P0_PB_MSK) ||
            ((ctx->scan_results_p1[i] & P1_PB_MSK) != P1_PB_MSK))
        {
            keyboard_return.scan_return = key_in_row(ctx, (7 - i) * 8, ctx->scan_results_p0[i], ctx->scan_results_p1[i]);

            if (keyboard_return.scan_return != SCAN_RETURN_SUCCESS)
                return keyboard_return;
        }
    }

    // Key scan completed
    for (int i = MAX_KEY_ROLLOVER - 1; i > -1; i--)
    {
        if (ctx->buffer_new[i] != 0xFF)
        {
            bool exist = false;

            for (int j = 0; j < MAX_KEY_ROLLOVER; j++)
            {
                if (ctx->buffer_new[i] == ctx->buffer_old[j])
                {
                    exist = true;
                    break;
                }
            }

            if (!exist)
            {
                ctx->buffer_quantity++;
                ctx->buffer[ctx->buffer_quantity] = ctx->buffer_new[i];
                ctx->simultaneous_keys++;

                if (ctx->simultaneous_keys == 0)
                {
                    ctx->buffer_quantity = -1;
                    ctx->simultaneous_alphanumeric_keys_flag = true;
                    keyboard_return.scan_return = SCAN_RETURN_MULTIPLE_KEYS_WITHIN_ONE_SCAN;
                    return keyboard_return;
                }
            }
        }
    }

    if (ctx->buffer_quantity >= 0)
    {
        ctx->buffer_quantity--;
        keyboard_return.alpha_num = ctx->buffer[0];
        memmove(ctx->buffer, ctx->buffer + 1, sizeof(ctx->buffer) - 1);
    }
    else
    {
        keyboard_return.alpha_num = 0xFF;
    }

    memcpy(ctx->buffer_old, ctx->buffer_new, MAX_KEY_ROLLOVER);
    keyboard_return.non_alpha_flag_x = ctx->non_alpha_flag_x;
    keyboard_return.non_alpha_flag_y = ctx->non_alpha_flag_y;
    keyboard_return.scan_return = SCAN_RETURN_SUCCESS;
    return keyboard_return;
}


static enum scan_return key_in_row(struct keyboard_ctx* ctx, int index, uint32_t scan_result_p0, uint32_t scan_result_p1)
{
    for (int i = 0; i < 8; i++)
    {
        if (((scan_result_p0 & P0_PIN_MSK(portb_pins[7 - i])) == 0) &&
            ((scan_result_p1 & P1_PIN_MSK(portb_pins[7 - i])) == 0))
        {
            // KeyFound
            if (ctx->key_quantity == 0)
                return SCAN_RETURN_KEY_SHADOWING_DETECTED; 

            ctx->key_quantity--;
            ctx->buffer_new[ctx->key_quantity] = key_table[index];
        }

        index++;
    }

    return SCAN_RETURN_SUCCESS;
}

