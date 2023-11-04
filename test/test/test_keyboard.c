/*******************************************************************************
 *    INCLUDED FILES
 ******************************************************************************/

#include <stdint.h>
#include <stdio.h>
#include <string.h>

//-- unity: unit test framework
#include "unity.h"
 
//-- module being tested
#include "keyboard.h"
//-- mocked modules
 
/*******************************************************************************
 *    DEFINITIONS
 ******************************************************************************/
 
/*******************************************************************************
 *    PRIVATE TYPES
 ******************************************************************************/
 
/*******************************************************************************
 *    PRIVATE DATA
 ******************************************************************************/

static struct keyboard_ctx kbd_ctx;

static uint8_t pa;

static uint8_t pa_msk;
static uint8_t pb_msk;
 
/*******************************************************************************
 *    PRIVATE FUNCTIONS
 ******************************************************************************/

static void pa_cfg_output(void)
{
}

static void pb_cfg_input_pull_high(void)
{
}

static void pa_out_write(uint8_t value)
{
    pa = value;
}

static uint8_t pb_in_read(void)
{
    if (~pa & pa_msk)
        return pb_msk ^ 0xFF;

    return 0xFF;
}

/*******************************************************************************
 *    SETUP, TEARDOWN
 ******************************************************************************/
 
void setUp(void)
{
    pa = 0;
    pa_msk = 0;
    pb_msk = 0;
    memset(&kbd_ctx, 0, sizeof(kbd_ctx));

    const struct keyboard_init_data init =
    {
        .pa_cfg_output = pa_cfg_output,
        .pb_cfg_input_pull_high = pb_cfg_input_pull_high,
        .pa_out_write = pa_out_write,
        .pb_in_read = pb_in_read,
    };
    keyboard_init(&kbd_ctx, &init);
}
 
void tearDown(void)
{
}
 
/*******************************************************************************
 *    TESTS
 ******************************************************************************/
 
void test_no_keys(void)
{
    struct keyboard_return keyboard_return = keyboard_scan(&kbd_ctx);

    TEST_ASSERT_EQUAL(SCAN_RETURN_NO_ACTIVITY, keyboard_return.keyboard_scan_return);
}

void test_z_key(void)
{
    pa_msk = 0x02;
    pb_msk = 0x10;
    struct keyboard_return keyboard_return = keyboard_scan(&kbd_ctx);

    TEST_ASSERT_EQUAL(SCAN_RETURN_SUCCESS, keyboard_return.keyboard_scan_return);
    TEST_ASSERT_EQUAL_UINT8(0x1A, keyboard_return.alpha_num);
    TEST_ASSERT_EQUAL_UINT8(0, keyboard_return.non_alpha_flag_x);
    TEST_ASSERT_EQUAL_UINT8(0, keyboard_return.non_alpha_flag_y);
}

void test_f5_key(void)
{
    pa_msk = 0x01;
    pb_msk = 0x40;
    struct keyboard_return keyboard_return = keyboard_scan(&kbd_ctx);

    TEST_ASSERT_EQUAL(SCAN_RETURN_SUCCESS, keyboard_return.keyboard_scan_return);
    TEST_ASSERT_EQUAL_UINT8(0xFF, keyboard_return.alpha_num);
    TEST_ASSERT_EQUAL_UINT8(1 << 6, keyboard_return.non_alpha_flag_x);
    TEST_ASSERT_EQUAL_UINT8(0, keyboard_return.non_alpha_flag_y);
}

