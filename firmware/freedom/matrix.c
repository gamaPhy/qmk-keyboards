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
    uint8_t key_x = sensor_lookup_table[sensor_nums[row][col]][sensor_value];

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

    uint16_t actuation_point;
    bool rapid_trigger;
    
    if (kb_config.use_per_key_settings) {
        actuation_point = kb_config.per_key_actuation_settings[sensor_nums[row][col]].actuation_point_dmm * LOOKUP_TABLE_MULTIPLIER;
        rapid_trigger = kb_config.per_key_actuation_settings[sensor_nums[row][col]].rapid_trigger;
    } else {
        actuation_point = kb_config.global_actuation_settings.actuation_point_dmm * LOOKUP_TABLE_MULTIPLIER;
        rapid_trigger = kb_config.global_actuation_settings.rapid_trigger;
    }

    // don't release until switch is above the original acutation point, preventing multipress bug.
    // amount subtracted is dependent on how many steps are between each dmm value in the lookup table
    uint16_t release_point = actuation_point - 2;

    if (rapid_trigger) {
        if (previous_states[row][col]) {
            // while the key is pressed, keep track of the lowest point of the key in current_extremes.
            // if the key is raised above the lowest point by sensitivity_delta, 
            // or above the main release point, release the key.
            int release_threshold;
            if (kb_config.use_per_key_settings) {
                release_threshold = current_extremes[row][col] - kb_config.per_key_actuation_settings[sensor_nums[row][col]].rapid_trigger_release_sensitivity_dmm * LOOKUP_TABLE_MULTIPLIER;
            } else {
                release_threshold = current_extremes[row][col] - kb_config.global_actuation_settings.rapid_trigger_release_sensitivity_dmm * LOOKUP_TABLE_MULTIPLIER;
            }
            if (key_x <= release_threshold || key_x <= release_point) {
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
            // If the key is above the main release point, only consider the main actuation point,
            // and ignore the actuation threshhold associated with rapid trigger
            if (current_extremes[row][col] <= release_point) {
                if (key_x >= actuation_point) {
                    current_extremes[row][col] = key_x;
                    return previous_states[row][col] = true;
                }
            }

            // while the key is released, keep track of the highest point of the key in current_extremes.
            // if the key is pressed below the highest point by sensitivity_delta, actuate the key.
            // however, the key must also be past the main actuation point
            uint16_t actuate_threshold;
            if (kb_config.use_per_key_settings) {
                actuate_threshold = current_extremes[row][col] + kb_config.per_key_actuation_settings[sensor_nums[row][col]].rapid_trigger_press_sensitivity_dmm * LOOKUP_TABLE_MULTIPLIER;
            } else {
                actuate_threshold = current_extremes[row][col] + kb_config.global_actuation_settings.rapid_trigger_press_sensitivity_dmm * LOOKUP_TABLE_MULTIPLIER;
            }
            if (key_x >= actuate_threshold && key_x >= actuation_point) {
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
            return previous_states[row][col] = key_x > release_point;
        } else {
            return previous_states[row][col] = key_x >= actuation_point;
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
        } 
        else if (!kb_config.calibrated || !lookup_table_ready) {
            // analog keys don't operate
            current_row_value |= 0;
        }
        else if (pin_mode == ANALOG && !calibrating_sensors) {
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
