// Copyright 2023 gamaPhy (https://gamaphy.com/)
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <limits.h>

// A constant that makes sensor scaling equation work good enough without requiring manual calibration of base values.
// More robust way of calibrating base values of sensors may be needed in the future.
// Dependent on HE sensor, VCC of HE sensors, and ADC_VREF.
// THIS_CONSTANT = sensor_min_reading - sensor_base_val
// Sensor: SLSS49E, 4.4V VCC, 2.5V ADC_VREF
#define SENSOR_BASE_OFFSET_4V4_2V5 275
// Sensor: SLSS49E, 3.3V VCC, 3.3V ADC_VREF
#define PICO_SENSOR_BASE_OFFSET 175

// A sensor at X_MIN_mm is when the switch is completely released. A sensor at X_MAX_mm is completely pressed.
#define X_MIN_mm (float)0
// 4.1mm is the greatest displacement in mm that a magnetic switch can be pressed according to Gateron datasheet
#define X_MAX_mm (float)4.1
// The maximum value that a key press can return from the lookup table.
// Units of dmm (decimillimetre), so a KEY_MAX_dmm value of 40 would mean that the greatest key press to appear in the lookup table is 4mm.
#define KEY_MAX_dmm 40

#define ADC_RESOLUTION 12
#define MAX_ADC_READING (1 << ADC_RESOLUTION) - 1
// the minimum range that sensor bounds should be after calibration
#define MIN_SENSOR_BOUND_RANGE 300

// corresponds to size of kb_config_t in freedom.h
// as of QMK commit 713427c, this value can be determined by setting it to something higher than you know kb_config_t will be, then reading the actual block size returned with QMK's error message, and use that value.
#define EECONFIG_KB_DATA_SIZE 128 
#define DEBUG_MATRIX_SCAN_RATE

#define RGBLIGHT_EFFECT_RAINBOW_SWIRL
#define RGBLIGHT_DEFAULT_MODE (RGBLIGHT_MODE_RAINBOW_SWIRL + 5)

#define WS2812_DI_PIN GP12
#define WS2812_PIO_USE_PIO0
#define RGBLED_NUM 10

// GPIO corresponding to Rasberry Pi Pico LED
#define PICO_LED GP25

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
