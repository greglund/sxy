/**
 * Copyright (c) 2014 - 2021, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

// Modifications for SXY project Copyright © 2023 Greg Lund


#include "app_error.h"
#include "app_timer.h"
#include "ble.h"
#include "ble_bas.h"
#include "ble_dis.h"
#include "ble_hids.h"
#include "ble_advertising.h"
#include "ble_advdata.h"
#include "ble_conn_params.h"
#include "ble_srv_common.h"
#include "keyboard.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "nrf_ble_gatt.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "peer_manager.h"
#include "peer_manager_handler.h"

#include <stdlib.h>
#include <string.h>


#define NRF_LOG_MODULE_NAME     main
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();


#define APP_BLE_OBSERVER_PRIO   3
#define DEVICE_NAME             "SXY-64 Keyboard"
#define MANUFACTURER_NAME       "Me"
#define APP_ADV_INTERVAL        300                                     /**< The advertising interval (in units of 0.625 ms. This value corresponds to 187.5 ms). */

#define APP_ADV_DURATION        BLE_GAP_ADV_TIMEOUT_GENERAL_UNLIMITED   /**< The advertising duration (unlimited) in units of 10 milliseconds. */
#define APP_BLE_CONN_CFG_TAG    1                                       /**< A tag identifying the SoftDevice BLE configuration. */
#define MIN_CONN_INTERVAL       MSEC_TO_UNITS(7.5, UNIT_1_25_MS)        /**< Minimum acceptable connection interval (7.5 milliseconds). */
#define MAX_CONN_INTERVAL       MSEC_TO_UNITS(7.5, UNIT_1_25_MS)        /**< Maximum acceptable connection interval (7.5 second). */
#define SLAVE_LATENCY           0                                       /**< Slave latency. */
#define CONN_SUP_TIMEOUT        MSEC_TO_UNITS(4000, UNIT_10_MS)         /**< Connection supervisory timeout (4 seconds). */

// 310F0000-D8C9-405D-8F6D-9CB237FDE8CC UUID basej
#define SXY_UUID_BASE           {0x31, 0x0F, 0x00, 0x00, 0xD8, 0xC9, 0x40, 0x5D, 0x8F, 0x6D, 0x9C, 0xB2, 0x37, 0xFD, 0xE8, 0xCC}
#define SXY_SERVICE_UUID        0x0001

#define FIRST_CONN_PARAMS_UPDATE_DELAY      APP_TIMER_TICKS(5000)       /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY       APP_TIMER_TICKS(30000)      /**< Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT        3                           /**< Number of attempts before giving up the connection parameter negotiation. */

#define SEC_PARAM_BOND          0                                       /**< Don't perform bonding. */
#define SEC_PARAM_MITM          0                                       /**< Man In The Middle protection not required. */
#define SEC_PARAM_LESC          0                                       /**< LE Secure Connections not enabled. */
#define SEC_PARAM_KEYPRESS      0                                       /**< Keypress notifications not enabled. */
#define SEC_PARAM_IO_CAPABILITIES   BLE_GAP_IO_CAPS_NONE                /**< No I/O capabilities. */
#define SEC_PARAM_OOB           0                                       /**< Out Of Band data not available. */
#define SEC_PARAM_MIN_KEY_SIZE  0                                       /**< Minimum encryption key size. */
#define SEC_PARAM_MAX_KEY_SIZE  0                                       /**< Maximum encryption key size. */

#define OUTPUT_REPORT_INDEX     0                                       /**< Index of Output Report. */
#define OUTPUT_REPORT_MAX_LEN   1                                       /**< Maximum length of Output Report. */
#define INPUT_REPORT_KEYS_INDEX 0                                       /**< Index of Input Report. */
#define OUTPUT_REPORT_BIT_MASK_CAPS_LOCK    0x02                        /**< CAPS LOCK bit in Output Report (based on 'LED Page (0x08)' of the Universal Serial Bus HID Usage Tables). */
#define INPUT_REP_REF_ID        0                                       /**< Id of reference to Keyboard Input Report. */
#define OUTPUT_REP_REF_ID       0                                       /**< Id of reference to Keyboard Output Report. */
#define FEATURE_REP_REF_ID      0                                       /**< ID of reference to Keyboard Feature Report. */
#define FEATURE_REPORT_MAX_LEN  2                                       /**< Maximum length of Feature Report. */
#define FEATURE_REPORT_INDEX    0                                       /**< Index of Feature Report. */

#define BASE_USB_HID_SPEC_VERSION   0x0101                              /**< Version number of base USB HID Specification implemented by this application. */

#define INPUT_REPORT_KEYS_MAX_LEN   8                                   /**< Maximum length of the Input Report characteristic. */

static void log_init(void);
static void timers_init(void);
static void power_management_init(void);
static void keyboard_module_init(void);
static void ble_stack_init(void);
static void gap_params_init(void);
static void gatt_init(void);
static void advertising_init(void);
static void services_init(void);
static void dis_init(void);
static void bas_init(void);
static void hids_init(void);
static void conn_params_init(void);
static void peer_manager_init(void);

static void advertising_start(void);

static void kbd_timer_handler(void* context);
static void ble_evt_handler(ble_evt_t const* evt, void* ctx);
static void gatt_evt_handler(nrf_ble_gatt_t* gatt, nrf_ble_gatt_evt_t const* evt);
static void on_adv_evt(ble_adv_evt_t ble_adv_evt);
static void on_bas_evt(ble_bas_t* bas, ble_bas_evt_t* evt);
static void on_hids_evt(ble_hids_t* hids, ble_hids_evt_t* evt);
static void service_error_handler(uint32_t nrf_error);
static void on_hid_rep_char_write(ble_hids_evt_t* evt);
static void on_conn_params_evt(ble_conn_params_evt_t* evt);
static void conn_params_error_handler(uint32_t nrf_error);
static void pm_evt_handler(pm_evt_t const* evt);


static const uint8_t report_map_data[] =
{
    0x05, 0x01,       // Usage Page (Generic Desktop)
    0x09, 0x06,       // Usage (Keyboard)
    0xA1, 0x01,       // Collection (Application)
    0x05, 0x07,       // Usage Page (Key Codes)

    // Modifier key input report
    0x19, 0xe0,       // Usage Minimum (224)
    0x29, 0xe7,       // Usage Maximum (231)
    0x15, 0x00,       // Logical Minimum (0)
    0x25, 0x01,       // Logical Maximum (1)
    0x75, 0x01,       // Report Size (1)
    0x95, 0x08,       // Report Count (8)
    0x81, 0x02,       // Input (Data, Variable, Absolute)

    // Reserved input report bytge
    0x95, 0x01,       // Report Count (1)
    0x75, 0x08,       // Report Size (8)
    0x81, 0x01,       // Input (Constant) reserved byte(1)

    // LED output report byte
    0x95, 0x05,       // Report Count (5)
    0x75, 0x01,       // Report Size (1)
    0x05, 0x08,       // Usage Page (Page# for LEDs)
    0x19, 0x01,       // Usage Minimum (1)
    0x29, 0x05,       // Usage Maximum (5)
    0x91, 0x02,       // Output (Data, Variable, Absolute), Led report
    0x95, 0x01,       // Report Count (1)
    0x75, 0x03,       // Report Size (3)
    0x91, 0x01,       // Output (Data, Variable, Absolute), Led report padding

    // Key input array (6 simultaneous keys)
    0x95, 0x06,       // Report Count (6)
    0x75, 0x08,       // Report Size (8)
    0x15, 0x00,       // Logical Minimum (0)
    0x25, 0x65,       // Logical Maximum (101)
    0x05, 0x07,       // Usage Page (Key codes)
    0x19, 0x00,       // Usage Minimum (0)
    0x29, 0x65,       // Usage Maximum (101)
    0x81, 0x00,       // Input (Data, Array) Key array(6 bytes)

    // Feature report bytes (2)
    0x09, 0x05,       // Usage (Vendor Defined)
    0x15, 0x00,       // Logical Minimum (0)
    0x26, 0xFF, 0x00, // Logical Maximum (255)
    0x75, 0x08,       // Report Size (8 bit)
    0x95, 0x02,       // Report Count (2)
    0xB1, 0x02,       // Feature (Data, Variable, Absolute)

    0xC0              // End Collection (Application)
};

static const ble_uuid128_t sxy_uuid_base =
{
    .uuid128 = SXY_UUID_BASE,
};

static struct keyboard_ctx kbd_ctx;
APP_TIMER_DEF(kbd_timer);
NRF_SDH_BLE_OBSERVER(ble_observer, APP_BLE_OBSERVER_PRIO, ble_evt_handler, NULL);
NRF_BLE_GATT_DEF(gatt);                                                 /**< GATT module instance. */
BLE_ADVERTISING_DEF(advertising);                                       /**< Advertising module instance. */
BLE_BAS_DEF(bas);                                                       /**< Battery service instance. */
BLE_HIDS_DEF(hids,                                                      /**< Structure used to identify the HID service. */
             NRF_SDH_BLE_TOTAL_LINK_COUNT,
             INPUT_REPORT_KEYS_MAX_LEN,
             OUTPUT_REPORT_MAX_LEN,
             FEATURE_REPORT_MAX_LEN);

static uint8_t sxy_uuid_type;
static ble_uuid_t adv_uuids[] =
{
    {SXY_SERVICE_UUID, BLE_UUID_TYPE_UNKNOWN}, // UUID type will be set at runtime
    {BLE_UUID_HUMAN_INTERFACE_DEVICE_SERVICE, BLE_UUID_TYPE_BLE},
};
static ble_hids_inp_rep_init_t input_report_array[1];
static ble_hids_outp_rep_init_t output_report_array[1];
static ble_hids_feature_rep_init_t feature_report_array[1];
static bool in_boot_mode;


int main(void)
{
    log_init();
    timers_init();
    power_management_init();
    keyboard_module_init();
    ble_stack_init();
    gap_params_init();
    gatt_init();
    advertising_init();
    services_init();
    conn_params_init();
    peer_manager_init();

    advertising_start();
    NRF_LOG_INFO("Application started.");

    for (;;)
    {
        if (!NRF_LOG_PROCESS())
            nrf_pwr_mgmt_run();
    }

    return 0;
}

static void log_init(void)
{
    ret_code_t err_code;

    err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
}

static void timers_init(void)
{
    ret_code_t err_code;

    err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);
}

static void power_management_init(void)
{
    ret_code_t err_code;

    err_code = nrf_pwr_mgmt_init();
    APP_ERROR_CHECK(err_code);
}

static void keyboard_module_init(void)
{
    ret_code_t err_code;

    keyboard_init(&kbd_ctx);

    err_code = app_timer_create(&kbd_timer, APP_TIMER_MODE_REPEATED, kbd_timer_handler);
    APP_ERROR_CHECK(err_code);

    err_code = app_timer_start(kbd_timer, APP_TIMER_TICKS(1000/60), NULL);
    APP_ERROR_CHECK(err_code);
}

static void ble_stack_init(void)
{
    ret_code_t err_code;

    err_code = nrf_sdh_enable_request();
    APP_ERROR_CHECK(err_code);

    uint32_t ram_start = 0;
    err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_sdh_ble_enable(&ram_start);
    APP_ERROR_CHECK(err_code);

    err_code = sd_ble_uuid_vs_add(&sxy_uuid_base, &sxy_uuid_type);
    APP_ERROR_CHECK(err_code);
}

static void gap_params_init(void)
{
    ret_code_t err_code;
    ble_gap_conn_params_t gap_conn_params = {0};
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    err_code = sd_ble_gap_device_name_set(
        &sec_mode, (const uint8_t*) DEVICE_NAME, strlen(DEVICE_NAME));
    APP_ERROR_CHECK(err_code);

    err_code = sd_ble_gap_appearance_set(BLE_APPEARANCE_HID_KEYBOARD);
    APP_ERROR_CHECK(err_code);

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout = CONN_SUP_TIMEOUT;

    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}

static void gatt_init(void)
{
    ret_code_t err_code = nrf_ble_gatt_init(&gatt, gatt_evt_handler);
    APP_ERROR_CHECK(err_code);
}

static void advertising_init(void)
{
    ret_code_t err_code;
    ble_advertising_init_t init = {0};

    adv_uuids[0].type = sxy_uuid_type;

    init.advdata.name_type = BLE_ADVDATA_FULL_NAME;
    init.advdata.include_appearance = true;
    init.advdata.flags = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;

    init.srdata.uuids_complete.uuid_cnt =
        sizeof(adv_uuids) / sizeof(adv_uuids[0]);
    init.srdata.uuids_complete.p_uuids = adv_uuids;

    init.config.ble_adv_fast_enabled = true;
    init.config.ble_adv_fast_interval = APP_ADV_INTERVAL;
    init.config.ble_adv_fast_timeout = APP_ADV_DURATION;

    init.evt_handler = on_adv_evt;

    err_code = ble_advertising_init(&advertising, &init);
    APP_ERROR_CHECK(err_code);

    ble_advertising_conn_cfg_tag_set(&advertising, APP_BLE_CONN_CFG_TAG);
}

static void services_init(void)
{
    dis_init();
    bas_init();
    hids_init();
}

static void dis_init(void)
{
    ret_code_t err_code;
    ble_dis_init_t dis_init = {0};

    ble_srv_ascii_to_utf8(&dis_init.manufact_name_str, (char*) MANUFACTURER_NAME);
    dis_init.dis_char_rd_sec = SEC_OPEN;

    err_code = ble_dis_init(&dis_init);
    APP_ERROR_CHECK(err_code);

}

static void bas_init(void)
{
    ret_code_t err_code;
    ble_bas_init_t bas_init = {0};

    bas_init.bl_cccd_wr_sec = SEC_OPEN;
    bas_init.bl_rd_sec = SEC_OPEN;
    bas_init.bl_report_rd_sec = SEC_OPEN;
    bas_init.evt_handler = on_bas_evt;

    err_code = ble_bas_init(&bas, &bas_init);
    APP_ERROR_CHECK(err_code);
}

static void hids_init(void)
{
    ret_code_t err_code;
    ble_hids_init_t hids_init_obj;
    ble_hids_inp_rep_init_t* input_report;
    ble_hids_outp_rep_init_t* output_report;
    ble_hids_feature_rep_init_t* feature_report;
    uint8_t hid_info_flags;

    in_boot_mode = false;
    memset((void*) input_report_array, 0, sizeof(ble_hids_inp_rep_init_t));
    memset((void*) output_report_array, 0, sizeof(ble_hids_outp_rep_init_t));
    memset((void*) feature_report_array, 0, sizeof(ble_hids_feature_rep_init_t));
    input_report = &input_report_array[INPUT_REPORT_KEYS_INDEX];
    input_report->max_len = INPUT_REPORT_KEYS_MAX_LEN;
    input_report->rep_ref.report_id = INPUT_REP_REF_ID;
    input_report->rep_ref.report_type = BLE_HIDS_REP_TYPE_INPUT;

    input_report->sec.cccd_wr = SEC_OPEN;
    input_report->sec.wr = SEC_OPEN;
    input_report->sec.rd = SEC_OPEN;

    output_report = &output_report_array[OUTPUT_REPORT_INDEX];
    output_report->max_len = OUTPUT_REPORT_MAX_LEN;
    output_report->rep_ref.report_id = OUTPUT_REP_REF_ID;
    output_report->rep_ref.report_type = BLE_HIDS_REP_TYPE_OUTPUT;

    output_report->sec.wr = SEC_OPEN;
    output_report->sec.rd = SEC_OPEN;

    feature_report = &feature_report_array[FEATURE_REPORT_INDEX];
    feature_report->max_len = FEATURE_REPORT_MAX_LEN;
    feature_report->rep_ref.report_id = FEATURE_REP_REF_ID;
    feature_report->rep_ref.report_type = BLE_HIDS_REP_TYPE_FEATURE;

    feature_report->sec.rd = SEC_OPEN;
    feature_report->sec.wr = SEC_OPEN;

    hid_info_flags =
        HID_INFO_FLAG_REMOTE_WAKE_MSK | HID_INFO_FLAG_NORMALLY_CONNECTABLE_MSK;

    memset(&hids_init_obj, 0, sizeof(hids_init_obj));

    hids_init_obj.evt_handler = on_hids_evt;
    hids_init_obj.error_handler = service_error_handler;
    hids_init_obj.is_kb = true;
    hids_init_obj.is_mouse = false;
    hids_init_obj.inp_rep_count = 1;
    hids_init_obj.p_inp_rep_array = input_report_array;
    hids_init_obj.outp_rep_count = 1;
    hids_init_obj.p_outp_rep_array = output_report_array;
    hids_init_obj.feature_rep_count = 1;
    hids_init_obj.p_feature_rep_array = feature_report_array;
    hids_init_obj.rep_map.data_len = sizeof(report_map_data);
    hids_init_obj.rep_map.p_data = (uint8_t*) report_map_data;
    hids_init_obj.hid_information.bcd_hid = BASE_USB_HID_SPEC_VERSION;
    hids_init_obj.hid_information.b_country_code = 0;
    hids_init_obj.hid_information.flags = hid_info_flags;
    hids_init_obj.included_services_count = 0;
    hids_init_obj.p_included_services_array = NULL;

    hids_init_obj.rep_map.rd_sec = SEC_OPEN;
    hids_init_obj.hid_information.rd_sec = SEC_OPEN;

    hids_init_obj.boot_kb_inp_rep_sec.cccd_wr = SEC_OPEN;
    hids_init_obj.boot_kb_inp_rep_sec.rd = SEC_OPEN;

    hids_init_obj.boot_kb_outp_rep_sec.rd = SEC_OPEN;
    hids_init_obj.boot_kb_outp_rep_sec.wr = SEC_OPEN;

    hids_init_obj.protocol_mode_rd_sec = SEC_OPEN;
    hids_init_obj.protocol_mode_wr_sec = SEC_OPEN;
    hids_init_obj.ctrl_point_wr_sec = SEC_OPEN;

    err_code = ble_hids_init(&hids, &hids_init_obj);
    APP_ERROR_CHECK(err_code);
}

static void conn_params_init(void)
{
    ret_code_t err_code;
    ble_conn_params_init_t cp_init = {0};

    cp_init.p_conn_params = NULL;
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    cp_init.next_conn_params_update_delay = NEXT_CONN_PARAMS_UPDATE_DELAY;
    cp_init.max_conn_params_update_count = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.start_on_notify_cccd_handle = BLE_CONN_HANDLE_INVALID;
    cp_init.disconnect_on_fail = false;
    cp_init.evt_handler = on_conn_params_evt;
    cp_init.error_handler = conn_params_error_handler;

    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}

static void peer_manager_init(void)
{
    // ble_gap_sec_params_t sec_param = {0};
    ret_code_t err_code;

    err_code = pm_init();
    APP_ERROR_CHECK(err_code);

    // Security parameters to be used for all security procedures.
    // sec_param.bond = SEC_PARAM_BOND;
    // sec_param.mitm = SEC_PARAM_MITM;
    // sec_param.lesc = SEC_PARAM_LESC;
    // sec_param.keypress = SEC_PARAM_KEYPRESS;
    // sec_param.io_caps = SEC_PARAM_IO_CAPABILITIES;
    // sec_param.oob = SEC_PARAM_OOB;
    // sec_param.min_key_size = SEC_PARAM_MIN_KEY_SIZE;
    // sec_param.max_key_size = SEC_PARAM_MAX_KEY_SIZE;

    // err_code = pm_sec_params_set(&sec_param);
    // APP_ERROR_CHECK(err_code);

    err_code = pm_register(pm_evt_handler);
    APP_ERROR_CHECK(err_code);
}

static void advertising_start(void)
{
    ret_code_t err_code;

    err_code = ble_advertising_start(&advertising, BLE_ADV_MODE_FAST);
    APP_ERROR_CHECK(err_code);
}

static void kbd_timer_handler(void* context)
{
    struct keyboard_return keyboard_return = keyboard_scan(&kbd_ctx);

    switch (keyboard_return.scan_return)
    {
        case SCAN_RETURN_SUCCESS:
            NRF_LOG_DEBUG("kbd_timer_handler: alpha_num: %x, non_alpha_flag_x: %x, non_alpha_flag_y: %x", 
                    keyboard_return.alpha_num,
                    keyboard_return.non_alpha_flag_x,
                    keyboard_return.non_alpha_flag_y);
            break;

        case SCAN_RETURN_NO_ACTIVITY:
            break;

        case SCAN_RETURN_AWAITING_NO_ACTIVITY:
            NRF_LOG_DEBUG("kbd_timer_handler: SCAN_RETURN_AWAITING_NO_ACTIVITY");
            break;

        case SCAN_RETURN_KEY_SHADOWING_DETECTED:
            NRF_LOG_DEBUG("kbd_timer_handler: SCAN_RETURN_KEY_SHADOWING_DETECTED");
            break;

        case SCAN_RETURN_MULTIPLE_KEYS_WITHIN_ONE_SCAN:
            NRF_LOG_DEBUG("kbd_timer_handler: SCAN_RETURN_MULTIPLE_KEYS_WITHIN_ONE_SCAN");
            break;

        default:
            // Should not happen
            NRF_LOG_INFO("kbd_timer_handler: unknown code %d", (int) keyboard_return.scan_return);
            break;
    }
}

static void ble_evt_handler(ble_evt_t const* evt, void* ctx)
{
    UNUSED_PARAMETER(ctx);
    ret_code_t err_code;

    switch (evt->header.evt_id)
    {
        case BLE_GAP_EVT_PHY_UPDATE_REQUEST:
            {
                NRF_LOG_DEBUG("PHY update request.");
                ble_gap_phys_t const phys =
                {
                    .rx_phys = BLE_GAP_PHY_AUTO,
                    .tx_phys = BLE_GAP_PHY_AUTO,
                };
                err_code = sd_ble_gap_phy_update(evt->evt.gap_evt.conn_handle, &phys);
                APP_ERROR_CHECK(err_code);
            }
            break;

        default:
            break;
    }
}

static void gatt_evt_handler(nrf_ble_gatt_t* gatt,
                             nrf_ble_gatt_evt_t const* evt)
{
    UNUSED_PARAMETER(gatt);
    UNUSED_PARAMETER(evt);
}

static void on_adv_evt(ble_adv_evt_t ble_adv_evt)
{
    UNUSED_PARAMETER(ble_adv_evt);
}

static void on_bas_evt(ble_bas_t* bas, ble_bas_evt_t* evt)
{
    UNUSED_PARAMETER(bas);
    UNUSED_PARAMETER(evt);
}

static void on_hids_evt(ble_hids_t* hids, ble_hids_evt_t* evt)
{
    UNUSED_PARAMETER(hids);

    switch (evt->evt_type)
    {
        case BLE_HIDS_EVT_BOOT_MODE_ENTERED:
            in_boot_mode = true;
            break;

        case BLE_HIDS_EVT_REPORT_MODE_ENTERED:
            in_boot_mode = false;
            break;

        case BLE_HIDS_EVT_REP_CHAR_WRITE:
            on_hid_rep_char_write(evt);
            break;

        case BLE_HIDS_EVT_NOTIF_ENABLED:
            break;

        default:
            // No implementation needed.
            break;
    }
}

static void service_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}

static void on_hid_rep_char_write(ble_hids_evt_t* evt)
{
    if (evt->params.char_write.char_id.rep_type == BLE_HIDS_REP_TYPE_OUTPUT)
    {
        // ret_code_t err_code;
        // uint8_t report_val;
        uint8_t report_index = evt->params.char_write.char_id.rep_index;

        if (report_index == OUTPUT_REPORT_INDEX)
        {

            // This code assumes that the output report is one byte long. Hence the following
            // static assert is made.
            // STATIC_ASSERT(OUTPUT_REPORT_MAX_LEN == 1);

            // err_code = ble_hids_outp_rep_get(&m_hids,
            //                                  report_index,
            //                                  OUTPUT_REPORT_MAX_LEN,
            //                                  0,
            //                                  m_conn_handle,
            //                                  &report_val);
            // APP_ERROR_CHECK(err_code);

            // if (!m_caps_on && ((report_val & OUTPUT_REPORT_BIT_MASK_CAPS_LOCK) != 0))
            // {
            //     // Caps Lock is turned On.
            //     NRF_LOG_INFO("Caps Lock is turned On!");
            //     err_code = bsp_indication_set(BSP_INDICATE_ALERT_3);
            //     APP_ERROR_CHECK(err_code);

            //     keys_send(sizeof(m_caps_on_key_scan_str), m_caps_on_key_scan_str);
            //     m_caps_on = true;
            // }
            // else if (m_caps_on && ((report_val & OUTPUT_REPORT_BIT_MASK_CAPS_LOCK) == 0))
            // {
            //     // Caps Lock is turned Off .
            //     NRF_LOG_INFO("Caps Lock is turned Off!");
            //     err_code = bsp_indication_set(BSP_INDICATE_ALERT_OFF);
            //     APP_ERROR_CHECK(err_code);

            //     keys_send(sizeof(m_caps_off_key_scan_str), m_caps_off_key_scan_str);
            //     m_caps_on = false;
            // }
            // else
            // {
            //     // The report received is not supported by this application. Do nothing.
            // }
        }
    }
}

static void on_conn_params_evt(ble_conn_params_evt_t* evt)
{
    ret_code_t err_code;

    NRF_LOG_DEBUG("on_conn_params: %u", evt->evt_type);

    if (evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
    {
        err_code = sd_ble_gap_disconnect(evt->conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
        APP_ERROR_CHECK(err_code);
    }
}

static void conn_params_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}

static void pm_evt_handler(pm_evt_t const* evt)
{
    pm_handler_on_pm_evt(evt);
    pm_handler_disconnect_on_sec_failure(evt);
    pm_handler_flash_clean(evt);
}
