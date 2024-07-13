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
// Used to create steps between each dmm value in the lookup table,
// so that there is no need for debounce despite noise on HE sensor outputs.
// The max value in the lookup table is LOOKUP_TABLE_MULTIPLER *
// KEY_MAX_dmm
#define LOOKUP_TABLE_MULTIPLIER 2

#define ADC_RESOLUTION 12
#define MAX_ADC_READING (1 << ADC_RESOLUTION) - 1
// the minimum range that sensor bounds should be after calibration
#define MIN_SENSOR_BOUND_RANGE 300

// corresponds to size of kb_config_t in freedom.h
#define EECONFIG_KB_DATA_SIZE 140

#define DEBUG_MATRIX_SCAN_RATE

// GPIO corresponding to Rasberry Pi Pico LED
#define PICO_LED GP25

// TODO: change to GP12
#define WS2812_DI_PIN GP13
#define WS2812_PIO_USE_PIO0
#define RGB_MATRIX_LED_COUNT 15

#define ENABLE_RGB_MATRIX_SOLID_COLOR // Static single hue, no speed support
#define ENABLE_RGB_MATRIX_ALPHAS_MODS
// Static dual hue, speed is hue for secondary hue
#define ENABLE_RGB_MATRIX_GRADIENT_UP_DOWN
// Static gradient top to bottom, speed controls how much gradient changes
#define ENABLE_RGB_MATRIX_GRADIENT_LEFT_RIGHT
// Static gradient left to right, speed controls how much gradient changes
#define ENABLE_RGB_MATRIX_BREATHING // Single hue brightness cycling animatio
#define ENABLE_RGB_MATRIX_BAND_SAT
// Single hue band fading saturation scrolling left to right
#define ENABLE_RGB_MATRIX_BAND_VAL
// Single hue band fading brightness scrolling left to right
#define ENABLE_RGB_MATRIX_BAND_PINWHEEL_SAT
// Single hue 3 blade spinning pinwheel fades saturation
#define ENABLE_RGB_MATRIX_BAND_PINWHEEL_VAL
// Single hue 3 blade spinning pinwheel fades brightness
#define ENABLE_RGB_MATRIX_BAND_SPIRAL_SAT
// Single hue spinning spiral fades saturation
#define ENABLE_RGB_MATRIX_BAND_SPIRAL_VAL
// Single hue spinning spiral fades brightness
#define ENABLE_RGB_MATRIX_CYCLE_ALL
// Full keyboard solid hue cycling through full gradient
#define ENABLE_RGB_MATRIX_CYCLE_LEFT_RIGHT
// Full gradient scrolling left to right
#define ENABLE_RGB_MATRIX_CYCLE_UP_DOWN
// Full gradient scrolling top to bottom
#define ENABLE_RGB_MATRIX_CYCLE_OUT_IN , // Full gradient scrolling out to in
#define ENABLE_RGB_MATRIX_CYCLE_OUT_IN_DUAL
// Full dual gradients scrolling out to in
#define ENABLE_RGB_MATRIX_RAINBOW_MOVING_CHEVRON
// Full gradient Chevron shapped scrolling left to right
#define ENABLE_RGB_MATRIX_CYCLE_PINWHEEL
// Full gradient spinning pinwheel around center of keyboard
#define ENABLE_RGB_MATRIX_CYCLE_SPIRAL
// Full gradient spinning spiral around center of keyboard
#define ENABLE_RGB_MATRIX_DUAL_BEACON
// Full gradient spinning around center of keyboard
#define ENABLE_RGB_MATRIX_RAINBOW_BEACON
// Full tighter gradient spinning around center of keyboard
#define ENABLE_RGB_MATRIX_RAINBOW_PINWHEELS
// Full dual gradients spinning two halfs of keyboard
#define ENABLE_RGB_MATRIX_FLOWER_BLOOMING
// Full tighter gradient of first half scrolling left to right and second
// half scrolling right to left
#define ENABLE_RGB_MATRIX_RAINDROPS
// Randomly changes a single key's hue
#define ENABLE_RGB_MATRIX_JELLYBEAN_RAINDROPS
// Randomly changes a single key's hue and saturation
#define ENABLE_RGB_MATRIX_HUE_BREATHING
// Hue shifts up a slight ammount at the same time, then shifts back
#define ENABLE_RGB_MATRIX_HUE_PENDULUM
// Hue shifts up a slight ammount in a wave to the right, then back to the
// left
#define ENABLE_RGB_MATRIX_HUE_WAVE
// Hue shifts up a slight ammount and then back down in a wave to the
// right
#define ENABLE_RGB_MATRIX_PIXEL_FRACTAL
// Single hue fractal filled keys pulsing horizontally out to edges
#define ENABLE_RGB_MATRIX_PIXEL_FLOW
// Pulsing RGB flow along LED wiring with random hues
#define ENABLE_RGB_MATRIX_PIXEL_RAIN     // Randomly light keys with random hues
#define ENABLE_RGB_MATRIX_TYPING_HEATMAP // How hot is your WPM!
#define ENABLE_RGB_MATRIX_DIGITAL_RAIN   // That famous computer simulation
#define ENABLE_RGB_MATRIX_SOLID_REACTIVE_SIMPLE
// Pulses keys hit to hue & value then fades value out
#define ENABLE_RGB_MATRIX_SOLID_REACTIVE
// Static single hue, pulses keys hit to shifted hue then fades to current
// hue
#define ENABLE_RGB_MATRIX_SOLID_REACTIVE_WIDE
// Hue & value pulse near a single key hit then fades value out
#define ENABLE_RGB_MATRIX_SOLID_REACTIVE_MULTIWIDE
// Hue & value pulse near multiple key hits then fades value out
#define ENABLE_RGB_MATRIX_SOLID_REACTIVE_CROSS
// Hue & value pulse the same column and row of a single key hit then
// fades value out
#define ENABLE_RGB_MATRIX_SOLID_REACTIVE_MULTICROSS
// Hue & value pulse the same column and row of multiple key hits then
// fade value out
#define ENABLE_RGB_MATRIX_SOLID_REACTIVE_NEXUS
// Hue & value pulse away on the same column and row of a single key hit
// then fades value out
#define ENABLE_RGB_MATRIX_SOLID_REACTIVE_MULTINEXUS
// Hue & value pulse away on the same column and row of multiple key hits
// then fades value out
#define ENABLE_RGB_MATRIX_SPLASH
// Full gradient & value pulse away from a single key hit then fades value
// out
#define ENABLE_RGB_MATRIX_MULTISPLASH
// Full gradient & value pulse away from multiple key hits then fades
// value out
#define ENABLE_RGB_MATRIX_SOLID_SPLASH
// Hue & value pulse away from a single key hit then fades value out
#define ENABLE_RGB_MATRIX_SOLID_MULTISPLASH
// Hue & value pulse away from multiple key hits then fades value out
#define ENABLE_RGB_MATRIX_STARLIGHT
// LEDs turn on and off at random at varying brightness, maintaining user
// set color
#define ENABLE_RGB_MATRIX_STARLIGHT_DUAL_HUE
// LEDs turn on and off at random at varying brightness, modifies user set
// hue by +- 30
#define ENABLE_RGB_MATRIX_STARLIGHT_DUAL_SAT
// LEDs turn on and off at random at varying brightness, modifies user set
// saturation by +- 30
#define ENABLE_RGB_MATRIX_RIVERFLOW
// Modification to breathing animation, offset's animation depending on
// key location to simulate a river flowing
#define ENABLE_RGB_MATRIX_EFFECT_MAX

// number of milliseconds to wait until rgb automatically turns off
#define RGB_MATRIX_TIMEOUT 0

// #define RGB_MATRIX_SLEEP // turn off effects when suspended
// limits the number of LEDs to process in an animation per task run
// (increases keyboard responsiveness)
#define RGB_MATRIX_LED_PROCESS_LIMIT (RGB_MATRIX_LED_COUNT + 4) / 5

// limits in milliseconds how frequently an animation will update the LEDs.
// 16 (16ms) is equivalent to limiting to 60fps (increases keyboard
// responsiveness)
#define RGB_MATRIX_LED_FLUSH_LIMIT 32

// Max possible value is 255
// maximum brightness is set to 255
// Sets the default enabled state, if none has been set
#define RGB_MATRIX_MAXIMUM_BRIGHTNESS 255
#define RGB_MATRIX_DEFAULT_ON true
#define RGB_MATRIX_DEFAULT_MODE RGB_MATRIX_CYCLE_PINWHEEL
// Sets the default hue value, if none has been set
#define RGB_MATRIX_DEFAULT_HUE 15
// Sets the default saturation value, if none has been set
#define RGB_MATRIX_DEFAULT_SAT 240
// Sets the default brightness value, if none has been set
#define RGB_MATRIX_DEFAULT_VAL 190
// Sets the default animation speed, if none has been set
#define RGB_MATRIX_DEFAULT_SPD 20
#define RGB_TRIGGER_ON_KEYDOWN // Triggers RGB keypress events on key down. This
                               // makes RGB control feel more responsive. This
                               // may cause RGB to not function properly on some
                               // boards
