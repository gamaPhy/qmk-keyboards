// Copyright 2023 gamaPhy (https://gamaphy.com/)
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <limits.h>

// A constant that makes sensor scaling equation work good enough without
// requiring manual calibration of base values. More robust way of calibrating
// base values of sensors may be needed in the future. Dependent on HE sensor,
// VCC of HE sensors, and ADC_VREF. THIS_CONSTANT = sensor_min_reading -
// sensor_base_val Sensor: SLSS49E, 4.4V VCC, 2.5V ADC_VREF
#define SENSOR_BASE_OFFSET_4V4_2V5 275
// Sensor: SLSS49E, 3.3V VCC, 3.3V ADC_VREF
#define PICO_SENSOR_BASE_OFFSET 160
// Sensor: DRV5056A3, 3.3V VCC, 3.3V ADC_VREF
// https://www.desmos.com/calculator/pcnugcacfy
#define DRV5056A3_BASE_OFFSET 222

// A sensor at X_MIN_mm is when the switch is completely released. A sensor at
// X_MAX_mm is completely pressed.
#define X_MIN_mm (float)0
// 4.1mm is the greatest displacement in mm that a magnetic switch can be
// pressed according to Gateron datasheet
#define X_MAX_mm (float)4.1
// The maximum value that a key's press distance is measured.
// Units of dmm (decimillimetre), so a KEY_MAX_dmm value of 40 would mean that
// any key distance measured beyond 4mm would be considered 4mm
#define KEY_MAX_dmm 40
// The amount that the calculated lookup table values are multiplied by.
// Used to create steps between each dmm value in the lookup table.
// The range of values in the lookup table is LOOKUP_TABLE_MULTIPLER *
// KEY_MAX_dmm
#define LOOKUP_TABLE_MULTIPLIER 2

#define ADC_RESOLUTION 12
#define MAX_ADC_READING (1 << ADC_RESOLUTION) - 1
// the minimum range that sensor bounds should be after calibration
#define MIN_SENSOR_BOUND_RANGE 300

// corresponds to size of kb_config_t in freedom.h
#define EECONFIG_KB_DATA_SIZE 140
#define DEBUG_MATRIX_SCAN_RATE

#define RGB_MATRIX_DEFAULT_SPD 20
#define ENABLE_RGB_MATRIX_CYCLE_PINWHEEL
#define RGB_MATRIX_DEFAULT_MODE RGB_MATRIX_CYCLE_PINWHEEL

#define WS2812_DI_PIN GP12
#define WS2812_PIO_USE_PIO0
#define RGB_MATRIX_LED_COUNT 10

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
