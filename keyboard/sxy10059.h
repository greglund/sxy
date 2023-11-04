/**
 * Copyright (c) 2017 - 2021, Nordic Semiconductor ASA
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
#ifndef PCA10059_H
#define PCA10059_H

#ifdef __cplusplus
extern "C" {
#endif

#include "nrf_gpio.h"

// LED definitions for PCA10059
// Each LED color is considered a separate LED
#define LEDS_NUMBER    4

#define LED1_G         NRF_GPIO_PIN_MAP(0,6)
#define LED2_R         NRF_GPIO_PIN_MAP(0,8)
#define LED2_G         NRF_GPIO_PIN_MAP(1,9)
#define LED2_B         NRF_GPIO_PIN_MAP(0,12)

#define LED_1          LED1_G
#define LED_2          LED2_R
#define LED_3          LED2_G
#define LED_4          LED2_B

#define LEDS_ACTIVE_STATE 0

#define LEDS_LIST { LED_1, LED_2, LED_3, LED_4 }

#define LEDS_INV_MASK  LEDS_MASK

#define BSP_LED_0      LED_1
#define BSP_LED_1      LED_2
#define BSP_LED_2      LED_3
#define BSP_LED_3      LED_4

// There is only one button for the application
// as the second button is used for a RESET.
#define BUTTONS_NUMBER 1

#define BUTTON_1       NRF_GPIO_PIN_MAP(1,6)
#define BUTTON_PULL    NRF_GPIO_PIN_PULLUP

#define BUTTONS_ACTIVE_STATE 0

#define BUTTONS_LIST { BUTTON_1 }

#define BSP_BUTTON_0   BUTTON_1

#define BSP_SELF_PINRESET_PIN NRF_GPIO_PIN_MAP(0,19)

#define HWFC           true

// PA       DIN25 Pin   nRF52 Port
// PA0      13          P0.24
// PA1      19          P0.21
// PA2      18          P0.19
// PA3      17          P0.17
// PA4      16          P0.10
// PA5      15          P0.08
// PA6      14          P0.11
// PA7      20          P0.23
#define PA0             NRF_GPIO_PIN_MAP(0, 24)
#define PA1             NRF_GPIO_PIN_MAP(0, 21)
#define PA2             NRF_GPIO_PIN_MAP(0, 19)
#define PA3             NRF_GPIO_PIN_MAP(0, 17)
#define PA4             NRF_GPIO_PIN_MAP(0, 10)
#define PA5             NRF_GPIO_PIN_MAP(0, 8)
#define PA6             NRF_GPIO_PIN_MAP(0, 11)
#define PA7             NRF_GPIO_PIN_MAP(0, 23)

// PB       DIN25 Pin   nRF52 Port
// PB0      12          P0.22
// PB1      11          P0.20
// PB2      10          P0.30
// PB3      5           P0.07
// PB4      8           P0.09
// PB5      7           P0.12
// PB6      6           P0.06
// PB7      9           P0.31
#define PB0             NRF_GPIO_PIN_MAP(0, 22)
#define PB1             NRF_GPIO_PIN_MAP(0, 20)
#define PB2             NRF_GPIO_PIN_MAP(0, 30)
#define PB3             NRF_GPIO_PIN_MAP(0, 7)
#define PB4             NRF_GPIO_PIN_MAP(0, 9)
#define PB5             NRF_GPIO_PIN_MAP(0, 12)
#define PB6             NRF_GPIO_PIN_MAP(0, 6)
#define PB7             NRF_GPIO_PIN_MAP(0, 31)

// Modifier DIN25 Pin
// Restore  3           P0.05
// ShiftLck 21          P0.25
#define RESTORE         NRF_GPIO_PIN_MAP(0, 5)
#define SHIFT_LOCK      NRF_GPIO_PIN_MAP(0, 25)

// LED
// LShftLck 22          P1.00
#define LED_SHIFT_LOCK  NRF_GPIO_PIN_MAP(1, 0)



#ifdef __cplusplus
}
#endif

#endif // PCA10059_H
