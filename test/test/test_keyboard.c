/*******************************************************************************
 *    INCLUDED FILES
 ******************************************************************************/

#include "boards.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

//-- unity: unit test framework
#include "unity.h"
 
//-- module being tested
#include "keyboard.h"
//-- mocked modules
#include "mock_nrf_delay.h"
#include "mock_nrf_gpio.h"
 
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
static uint32_t p0_dir;
static uint32_t p0_in;
static uint32_t p0_out;
static uint32_t p1_dir;
static uint32_t p1_in;
static uint32_t p1_out;

 
/*******************************************************************************
 *    PRIVATE FUNCTIONS
 ******************************************************************************/

static NRF_GPIO_Type* port_decode(uint32_t pin)
{
    if (pin < 32)
        return NRF_P0;

    return NRF_P1;
}

static void _nrf_gpio_port_dir_output_set(NRF_GPIO_Type* port, uint32_t msk, int cmock_num_calls)
{
    if (port == NRF_P0)
        p0_dir = msk;
    else if (port == NRF_P1)
        p1_dir = msk;
    else
        TEST_ASSERT(false);
}

static void _nrf_gpio_cfg_input(uint32_t pin_number, nrf_gpio_pin_pull_t pull_config, int cmock_num_calls)
{
    p0_dir = p0_dir & ~(1 << (pin_number & 0x1F));
}

static void _nrf_gpio_port_out_clear(NRF_GPIO_Type* p_reg, uint32_t clr_mask, int cmock_num_calls)
{
    if (p_reg == NRF_P0)
        p0_out = p0_out & ~(clr_mask);
    else if (p_reg == NRF_P1)
        p1_out = p1_out & ~(clr_mask);
    else
        TEST_ASSERT(false);
}

static void _nrf_gpio_port_out_set(NRF_GPIO_Type* p_reg, uint32_t set_mask, int cmock_num_calls)
{
    if (p_reg == NRF_P0)
        p0_out = p0_out | set_mask;
    else if (p_reg == NRF_P1)
        p1_out = p1_out | set_mask;
    else
        TEST_ASSERT(false);
}

static uint32_t _nrf_gpio_port_in_read(NRF_GPIO_Type const* p_reg, int cmock_num_calls)
{
    if (p_reg == NRF_P0)
        return 0xFFFFFFFF;

    if (p_reg == NRF_P1)
        return 0xFFFFFFFF;

    TEST_ASSERT(false);
}

static uint32_t _nrf_gpio_port_in_read_z(NRF_GPIO_Type const* p_reg, int cmock_num_calls)
{
    uint32_t out;

    if (p_reg == NRF_P0)
    {
        out = p0_out;

        if (port_decode(PA1) == NRF_P0)
            out = out & (1 << (PA1 & 0x1F));
    }
    else if (p_reg == NRF_P1)
    {
        out = p1_out;

        if (port_decode(PA1) == NRF_P1)
            out = out & (1 << (PA1 & 0x1F));
    }
    else
    {
        TEST_ASSERT(false);
    }

    if ((out == 0) && (port_decode(PB4) == p_reg))
        out = 0xFFFFFFFF & ~(1 << (PB4 & 0x1F));
    else
        out = 0xFFFFFFFF;

    return out;
}
 
static uint32_t _nrf_gpio_port_in_read_f5(NRF_GPIO_Type const* p_reg, int cmock_num_calls)
{
    uint32_t out;

    if (p_reg == NRF_P0)
        out = p0_out & (1 << 24);
    else if (p_reg == NRF_P1)
        out = 0xFFFFFFFF;
    else
        TEST_ASSERT(false);

    if (out == 0)
        out = 0xFFFFFFFF & ~(1 << 6);
    else
        out = 0xFFFFFFFF;

    return out;
}

/*******************************************************************************
 *    SETUP, TEARDOWN
 ******************************************************************************/
 
void setUp(void)
{
    p0_dir = 0;
    p0_in = 0;
    p0_out = 0;
    p1_dir = 0;
    p1_in = 0;
    p1_out = 0;
    memset(&kbd_ctx, 0, sizeof(kbd_ctx));
    nrf_delay_us_Ignore();
    nrf_gpio_port_out_clear_StubWithCallback(_nrf_gpio_port_out_clear);
    nrf_gpio_port_dir_output_set_StubWithCallback(&_nrf_gpio_port_dir_output_set);
    nrf_gpio_port_out_set_StubWithCallback(_nrf_gpio_port_out_set);
    nrf_gpio_cfg_input_StubWithCallback(&_nrf_gpio_cfg_input);
    keyboard_init(&kbd_ctx);
}
 
void tearDown(void)
{
}
 
/*******************************************************************************
 *    TESTS
 ******************************************************************************/
 
void test_no_keys(void)
{
    nrf_gpio_port_in_read_StubWithCallback(_nrf_gpio_port_in_read);

    struct keyboard_return keyboard_return = keyboard_scan(&kbd_ctx);

    TEST_ASSERT_EQUAL(SCAN_RETURN_NO_ACTIVITY, keyboard_return.scan_return);
}

void test_z_key(void)
{
    nrf_gpio_port_in_read_StubWithCallback(_nrf_gpio_port_in_read_z);

    struct keyboard_return keyboard_return = keyboard_scan(&kbd_ctx);

    TEST_ASSERT_EQUAL(SCAN_RETURN_SUCCESS, keyboard_return.scan_return);
    TEST_ASSERT_EQUAL_UINT8(0x1A, keyboard_return.alpha_num);
    TEST_ASSERT_EQUAL_UINT8(0, keyboard_return.non_alpha_flag_x);
    TEST_ASSERT_EQUAL_UINT8(0, keyboard_return.non_alpha_flag_y);
}

void test_f5_key(void)
{
    nrf_gpio_port_in_read_StubWithCallback(_nrf_gpio_port_in_read_f5);

    struct keyboard_return keyboard_return = keyboard_scan(&kbd_ctx);

    TEST_ASSERT_EQUAL(SCAN_RETURN_SUCCESS, keyboard_return.scan_return);
    TEST_ASSERT_EQUAL_UINT8(0xFF, keyboard_return.alpha_num);
    TEST_ASSERT_EQUAL_UINT8(1 << 6, keyboard_return.non_alpha_flag_x);
    TEST_ASSERT_EQUAL_UINT8(0, keyboard_return.non_alpha_flag_y);
}

