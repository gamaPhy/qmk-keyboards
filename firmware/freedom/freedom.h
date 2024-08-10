#pragma once

#include "analog.h"
#include "quantum.h"

#include "helpers/kb_config.h"

enum custom_keycodes {
  KC_CALIBRATE = QK_KB,
  KC_TOGGLE_RAPID_TRIGGER,
  KC_ACTUATION_DEC,
  KC_ACTUATION_INC,
  NEW_QK_KB
};

_Static_assert(
    sizeof(kb_config_t) <= EECONFIG_KB_DATA_SIZE,
    "EECONFIG_KB_DATA_SIZE is too small to store keyboard configuration.");

extern bool bootup_calibrated;
extern uint8_t sensor_lookup_table[SENSOR_COUNT][MAX_ADC_READING];
extern sensor_bounds_t running_sensor_bounds[SENSOR_COUNT];
extern bool calibrating_sensors;
extern const pin_t direct_pins[MATRIX_ROWS][MATRIX_COLS];
