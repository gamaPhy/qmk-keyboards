// Copyright 2023 gamaPhy (https://gamaphy.com/)
// SPDX-License-Identifier: GPL-3.0-or-later

#include QMK_KEYBOARD_H

#include "sensor_read.h"

extern matrix_row_t raw_matrix[MATRIX_ROWS]; // raw values
extern matrix_row_t matrix[MATRIX_ROWS];     // debounced values
extern uint16_t     min1, max1, min2, max2, min3, max3;

uint8_t (*sensor_lookup_table)[MAX_ADC_READING];

void matrix_init_custom(void) {
    for (int row = 0; row < MATRIX_ROWS; row++) {
        for (int col = 0; col < MATRIX_COLS; col++) {
            pin_t           pin = direct_pins[row][col];
            pin_scan_mode_t pin_mode = pin_scan_modes[row][col];
            if (pin_mode == ANALOG) {
                palSetLineMode(pin, PAL_MODE_INPUT_ANALOG);
            } else if (pin_mode == DIGITAL) {
                setPinInputHigh(pin);
            }
        }
    }
}

bool scan_pin_analog(pin_t pin, uint8_t row, uint8_t col) {
    static uint16_t current_extremes[MATRIX_ROWS][MATRIX_COLS] = { 0 };
    static bool     previous_states[MATRIX_ROWS][MATRIX_COLS] = { 0 };

    uint16_t sensor_value = oversample(pin);
    // distance key is pressed down
    uint8_t key_x = sensor_lookup_table[sensor_num[row][col]][sensor_value];

    if (col == 0) {
        if (sensor_value > max1) {
            max1 = sensor_value;
        }
        if (sensor_value < min1) {
            min1 = sensor_value;
        }
    }
    if (col == 1) {
        if (sensor_value > max2) {
            max2 = sensor_value;
        }
        if (sensor_value < min2) {
            min2 = sensor_value;
        }
    }
    if (col == 2) {
        if (sensor_value > max3) {
            max3 = sensor_value;
        }
        if (sensor_value < min3) {
            min3 = sensor_value;
        }
    }

    uint16_t actuation_point_adc = kb_config.actuation_point_dmm;

    if (kb_config.rapid_trigger) {
        uint16_t sensitivity_delta = kb_config.rapid_trigger_sensitivity_dmm;
        if (previous_states[row][col]) {
            // while the key is pressed, keep track of the lowest point of the key in current_extremes.
            // if the key is raised above the lowest point by sensitivity_delta, release the key.
            uint16_t release_threshhold = current_extremes[row][col] - sensitivity_delta;
            if (key_x < release_threshhold) {
                current_extremes[row][col] = key_x;
                return previous_states[row][col] = false;
            }
            // if the key is pressed down farther, release_threshhold will be lower in subsequent scans
            if (key_x > current_extremes[row][col]) {
                current_extremes[row][col] = key_x;
            }
            // the key did not go above the release_threshhold, so it stays pressed
            return previous_states[row][col] = true;
        } else {
            // while the key is released, keep track of the highest point of the key in current_extremes.
            // if the key is pressed below the highest point by sensitivity_delta, actuate the key.
            // however, the key must also be past the main actuation point
            uint16_t actuate_threshhold = current_extremes[row][col] + sensitivity_delta;
            if (key_x > actuate_threshhold && key_x > actuation_point_adc) {
                current_extremes[row][col] = key_x;
                return previous_states[row][col] = true;
            }
            // if the key is raised farther, actuate_threshhold will be higher in subsequent scans
            if (key_x < current_extremes[row][col]) {
                current_extremes[row][col] = key_x;
            }
            // the key did not go below the actuate_threshhold, so it stays released
            return previous_states[row][col] = false;
        }
    } else {
        if (previous_states[row][col]) {
            // don't release until switch is above the original acutation point, preventing multipress bug
            return previous_states[row][col] = key_x >= actuation_point_adc - 2;
        } else {
            return previous_states[row][col] = key_x >= actuation_point_adc;
        }
    }
}

void matrix_read_cols_on_row(matrix_row_t current_matrix[], uint8_t current_row) {
    // Start with a clear matrix row
    matrix_row_t current_row_value = 0;

    matrix_row_t row_shifter = 1;
    for (uint8_t col_index = 0; col_index < MATRIX_COLS; col_index++, row_shifter <<= 1) {
        pin_t           pin = direct_pins[current_row][col_index];
        pin_scan_mode_t pin_mode = pin_scan_modes[current_row][col_index];
        if (pin_mode == DIGITAL) {
            current_row_value |= readPin(pin) ? 0 : row_shifter;
        } else if (pin_mode == ANALOG && !calibrating_sensors) {
            if (scan_pin_analog(pin, current_row, col_index)) {
                current_row_value |= row_shifter;
            }
        }
    }

    current_matrix[current_row] = current_row_value;
}

bool matrix_scan_custom(matrix_row_t current_matrix[]) {
    matrix_row_t new_matrix[MATRIX_ROWS] = { 0 };
    for (int row = 0; row < MATRIX_ROWS; row++) {
        matrix_read_cols_on_row(new_matrix, row);
    }

    bool changed = memcmp(current_matrix, new_matrix, sizeof(new_matrix)) != 0;
    if (changed) {
        memcpy(current_matrix, new_matrix, sizeof(new_matrix));
    }
    return changed;
}
