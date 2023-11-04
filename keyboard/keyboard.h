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

#if !defined(KEYBOARD_H_)
#define KEYBOARD_H_

#include <stdbool.h>
#include <stdint.h>


#if defined(__cplusplus)
extern "C"
{
#endif

#define MAX_KEY_ROLLOVER    3
enum scan_return
{
    SCAN_RETURN_SUCCESS,
    SCAN_RETURN_NO_ACTIVITY,
    SCAN_RETURN_CONTROL_PORT_ACTIVITY_DETECTED,
    SCAN_RETURN_KEY_SHADOWING_DETECTED,
    SCAN_RETURN_MULTIPLE_KEYS_WITHIN_ONE_SCAN,
    SCAN_RETURN_AWAITING_NO_ACTIVITY,
    SCAN_RETURNS,
};

struct keyboard_ctx
{
    uint32_t scan_results_p0[8];
    uint32_t scan_results_p1[8];
    uint8_t buffer_new[MAX_KEY_ROLLOVER];
    uint8_t buffer_old[MAX_KEY_ROLLOVER];
    uint8_t buffer[4];
    int buffer_quantity;
    uint8_t key_quantity;
    uint8_t non_alpha_flag_x;
    uint8_t non_alpha_flag_y;
    uint8_t simultaneous_keys;
    bool simultaneous_alphanumeric_keys_flag;
};

struct keyboard_return
{
    enum scan_return scan_return;
    uint8_t alpha_num;
    uint8_t non_alpha_flag_x;
    uint8_t non_alpha_flag_y;
};


void keyboard_init(struct keyboard_ctx* ctx);
struct keyboard_return keyboard_scan(struct keyboard_ctx* ctx);

#if defined(__cplusplus)
}
#endif
#endif // !defined(KEYBOARD_H_)

