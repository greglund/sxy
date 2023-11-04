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

#include "keyboard.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>


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

static enum keyboard_scan_return keyboard_key_in_row(struct keyboard_ctx* ctx, int index, uint32_t scan_result);
static uint8_t keyboard_rotate_left(uint8_t data);


void keyboard_init(struct keyboard_ctx* ctx, const struct keyboard_init_data* init)
{
    ctx->pa_cfg_output = init->pa_cfg_output;
    ctx->pb_cfg_input_pull_high = init->pb_cfg_input_pull_high;
    ctx->pa_out_write = init->pa_out_write;
    ctx->pb_in_read = init->pb_in_read;

    // Set port direction
    ctx->pa_cfg_output();
    ctx->pb_cfg_input_pull_high();

    memset(ctx->buffer_old, 0xFF, sizeof(ctx->buffer_old));
    memset(ctx->buffer, 0xFF, sizeof(ctx->buffer));
    ctx->buffer_quantity = -1;
}

struct keyboard_return keyboard_scan(struct keyboard_ctx* ctx)
{
    struct keyboard_return keyboard_return = {0};

    // Connect all Keyboard rows
    ctx->pa_out_write(0);

    // Check for port activity
    if (ctx->pb_in_read() == 0xFF)
    {
        ctx->simultaneous_alphanumeric_keys_flag = false;
        memset(ctx->buffer_old, 0xFF, sizeof(ctx->buffer_old));
        keyboard_return.keyboard_scan_return = SCAN_RETURN_NO_ACTIVITY;
        return keyboard_return;
    }

    // Wait for  all keys to be released before accepting new input
    if (ctx->simultaneous_alphanumeric_keys_flag)
    {
        keyboard_return.keyboard_scan_return = SCAN_RETURN_AWAITING_NO_ACTIVITY;
        return keyboard_return;
    }

    // Scan keyboard matrix
    uint8_t strobe = 0xFE;
    ctx->pa_out_write(strobe);
    ctx->scan_results[7] = ctx->pb_in_read();

    for (int i = 6; i > -1; i--)
    {
        strobe = keyboard_rotate_left(strobe);
        ctx->pa_out_write(strobe);
        ctx->scan_results[i] = ctx->pb_in_read();
    }

    // Initialize buffer, flags and max keys
    memset(ctx->buffer_new, 0xFF, sizeof(ctx->buffer_new));
    ctx->non_alpha_flag_y = 0;

    ctx->key_quantity = MAX_KEY_ROLLOVER;
    ctx->simultaneous_keys = 0xFE;

    // Check and flag non-alphanumeric keys
    ctx->non_alpha_flag_y = ((ctx->scan_results[6] ^ 0xFF) & 0x80) >> 1; // Left SHIFT key
    ctx->non_alpha_flag_y |= ((ctx->scan_results[0] ^ 0xFF) & 0xA4); // RUN STOP - C= - CTRL
    ctx->non_alpha_flag_y |= ((ctx->scan_results[1] ^ 0xFF) & 0x18); // Right SHIFT - CLR HOME

    ctx->non_alpha_flag_x = ctx->scan_results[7] ^ 0xFF;    // The rest
    
    // Check for pressed keys
    for (int i = 7; i > -1; i--)
    {
        if (ctx->scan_results[i] != 0xFF)
        {
            keyboard_return.keyboard_scan_return = keyboard_key_in_row(ctx, (7 - i) * 8, ctx->scan_results[i]);

            if (keyboard_return.keyboard_scan_return != SCAN_RETURN_SUCCESS)
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
                    keyboard_return.keyboard_scan_return = SCAN_RETURN_MULTIPLE_KEYS_WITHIN_ONE_SCAN;
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
    keyboard_return.keyboard_scan_return = SCAN_RETURN_SUCCESS;
    return keyboard_return;
}


static enum keyboard_scan_return keyboard_key_in_row(struct keyboard_ctx* ctx, int index, uint32_t scan_result)
{
    for (int i = 0; i < 8; i++)
    {
        if ((scan_result & (1 << (7 - i))) == 0)
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

static uint8_t keyboard_rotate_left(uint8_t data)
{
    return ((data >> 7) & 0x01) | (data << 1);
}
