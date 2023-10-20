// Copyright 2023 gamaPhy (https://gamaphy.com/)
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <limits.h>

#define ADC_RESOLUTION 12
#define MAX_ADC_READING (1 << ADC_RESOLUTION) - 1
// transforms a decimal value to an int. 
// Ex: The decimal value of 9.123 is 0.123 
#define DECIMAL_TO_INT(val) (val - (float)(int)val) * (float)INT_MAX 
#define INT_TO_DECIMAL(val) (float)val / (float)INT_MAX

// corresponds to size of kb_config_t in freedom.h
// (as of QMK commit 713427c, this value can be determined by setting it to something higher than you know kb_config_t will be, then reading the actual block size returned with QMK's error message, and use that value.
#define EECONFIG_KB_DATA_SIZE 124 
#define DEBUG_MATRIX_SCAN_RATE
#define BOOTMAGIC_LITE_ROW 1
#define BOOTMAGIC_LITE_COLUMN 0

#define RGBLIGHT_EFFECT_RAINBOW_SWIRL
#define RGBLIGHT_DEFAULT_MODE (RGBLIGHT_MODE_RAINBOW_SWIRL + 5)

#define WS2812_PIO_USE_PIO0
// #define WS2812_PWM_DRIVER PWMD3
// #define WS2812_PWM_CHANNEL 4
// #define WS2812_PWM_PAL_MODE 2
// #define WS2812_DMA_STREAM STM32_DMA1_STREAM2
// #define WS2812_DMA_CHANNEL 5

// #define WS2812_TIMING 1250
// #define WS2812_T0H 400 
// #define WS2812_T1H 800
// #define WS2812_T0L 2050
// #define WS2812_T1L 1650
// #define WS2812_RES_US 100
