// Copyright 2023 gamaPhy (https://gamaphy.com/)
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "quantum.h"
#include "analog.h"

enum custom_keycodes {
    KC_CALIBRATE = QK_KB,
    KC_TOGGLE_RAPID_TRIGGER,
    KC_ACTUATION_DEC,
    KC_ACTUATION_INC,
    NEW_QK_KB
};

typedef enum {
    NO_SCAN,
    DIGITAL,
    ANALOG
} pin_scan_mode_t;

typedef struct {
    uint16_t min;
    uint16_t max;
} sensor_bounds_t;

// Parameters of sensor scaling equation:
// y(x) = (a/(x+b)^3) + base_value
// where `base_value` is the value output from a HE sensor with no switches placed on the PCB.
typedef struct {
    int32_t a;
    int16_t b;
    // The actual decimal value of `b` is derived by doing `(float)b_fractional_component / (float)INT_MAX`
    int32_t b_fractional_component;
    uint16_t base_value;
} sensor_scaling_params_t;

typedef struct {
    bool calibrated;
    bool rapid_trigger;
    uint8_t actuation_point_mm;
    uint8_t release_point_mm;
    uint8_t rapid_trigger_sensitivity_mm;
    sensor_bounds_t matrix_sensor_bounds[MATRIX_ROWS][MATRIX_COLS];
    sensor_scaling_params_t matrix_scaling_params[MATRIX_ROWS][MATRIX_COLS];
} kb_config_t;


extern kb_config_t kb_config;
extern bool calibrating_sensors;
extern const pin_t direct_pins[MATRIX_ROWS][MATRIX_COLS];
extern const pin_scan_mode_t pin_scan_modes[MATRIX_ROWS][MATRIX_COLS];
extern const int sensor_num[MATRIX_ROWS][MATRIX_COLS];


_Static_assert(sizeof(kb_config_t) <= EECONFIG_KB_DATA_SIZE, "EECONFIG_KB_DATA_SIZE is too small to store keyboard configuration.");

