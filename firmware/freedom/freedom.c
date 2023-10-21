// Copyright 2023 gamaPhy (https://gamaphy.com/)
// SPDX-License-Identifier: GPL-3.0-or-later

#include <math.h>
#include <limits.h>

#include "freedom.h"

bool calibrating_sensors = false;

kb_config_t kb_config;

const pin_t direct_pins[MATRIX_ROWS][MATRIX_COLS] = DIRECT_PINS;
const pin_scan_mode_t pin_scan_modes[MATRIX_ROWS][MATRIX_COLS] = PIN_SCAN_MODES;
const int sensor_num[MATRIX_ROWS][MATRIX_COLS] = SENSOR_NUM;

// There are 40 (4mm/0.1mm) distances that need to be represented 
// To get the real distance a key is pressed, must divide the uint8_t value by 10
extern uint8_t (*sensor_lookup_table)[MAX_ADC_READING];

uint16_t min1, max1, min2, max2, min3, max3;

void eeconfig_init_kb(void) {
    kb_config.rapid_trigger = false;
    kb_config.actuation_point_mm = 20;
    kb_config.release_point_mm = 16;
    kb_config.rapid_trigger_sensitivity_mm = 10;
    for (int row = 0; row < MATRIX_ROWS; row++) {
        for (int col = 0; col < MATRIX_COLS; col++) {
            if (pin_scan_modes[row][col] == ANALOG) {
                kb_config.matrix_sensor_bounds[row][col].min = 2200;
                kb_config.matrix_sensor_bounds[row][col].max = 3000;
                kb_config.matrix_scaling_params[row][col].a = 0;
                kb_config.matrix_scaling_params[row][col].b = 0;
                kb_config.matrix_scaling_params[row][col].b_decimal = 0;
                kb_config.matrix_scaling_params[row][col].base_value = 0;
            }
        }
    }
    eeconfig_update_kb_datablock(&kb_config);
}

// The greatest displacement in mm that a magnetic switch can be pressed according to Gateron datasheet
// A sensor at X_MAX is not pressed. A sensor at X_MIN is pressed.
#define X_MIN 0
#define X_MAX (float)4.1
#define RATIO(a, b, base) (float)a/((float)b - (float)base)
#define B_PARAM(sensor_at_x_max, sensor_at_x_min, base) (float)(X_MAX * (cbrt(RATIO(sensor_at_x_max, sensor_at_x_min, base)) / (1.0 - cbrt(RATIO(sensor_at_x_max, sensor_at_x_min, base)))))
#define A_PARAM(sensor_at_x_max, sensor_at_x_min, base) ((float)sensor_at_x_min - (float)base) * pow(B_PARAM(sensor_at_x_max, sensor_at_x_min, base), 3)

// Computes and stores the `a` and `b` parameters of the best-fit scaling equation. 
// b = X_MAX(cbrt(r))/(1-cbrt(r))
// a = (sensor_reading@X_MIN - base_value) * b^3
// where `r = sensor_reading@X_MAX / (sensor_reading@X_MIN - base_value)` 
void compute_sensor_scaling_params(void){
    for (int row = 0; row < MATRIX_ROWS; row++) {
        for (int col = 0; col < MATRIX_COLS; col++) {
            if (pin_scan_modes[row][col] == ANALOG) {
                int min = kb_config.matrix_sensor_bounds[row][col].min;
                int max = kb_config.matrix_sensor_bounds[row][col].max;
                int base_val = kb_config.matrix_scaling_params[row][col].base_value;

                kb_config.matrix_scaling_params[row][col].b = B_PARAM(min, max, base_val);
                kb_config.matrix_scaling_params[row][col].b_decimal = FRACTIONAL_COMPONENT_TO_INT(B_PARAM(min, max, base_val));
                kb_config.matrix_scaling_params[row][col].a = A_PARAM(min, max, base_val);

                dprintf("Sensor MIN: %i\n", (int) min);
                dprintf("Sensor MAX: %i\n", (int) max);
                dprintf("A: %li\n", kb_config.matrix_scaling_params[row][col].a);
                dprintf("B: %i\n", kb_config.matrix_scaling_params[row][col].b);
                dprintf("B decimal: %li / %i\n", kb_config.matrix_scaling_params[row][col].b_decimal, INT_MAX);
                dprintf("BASE: %i\n", kb_config.matrix_scaling_params[row][col].base_value);
            }
        }
    }
}

// stores the following calculation into each cell of the lookup table, and pads the rest with either X_MIN or X_MAX depending the side of the array
// (cbrt(a/sensor_reading) - b) * 10
// A multiplication of 10 is added to convert mm to mm/10
void create_lookup_table(void) {
    // memory had been previously allocated for the lookup table
    if (sensor_lookup_table != NULL) {
        free(sensor_lookup_table);
    }

    sensor_lookup_table = malloc(SENSOR_COUNT * sizeof(*sensor_lookup_table));
    
    if(sensor_lookup_table != NULL) {
        for (int row = 0; row < MATRIX_ROWS; row++) {
            for (int col = 0; col < MATRIX_COLS; col++) {
                if (pin_scan_modes[row][col] == ANALOG) {
                    float a = (float)kb_config.matrix_scaling_params[row][col].a;
                    float b = (float)kb_config.matrix_scaling_params[row][col].b + INT_TO_FRACTIONAL_COMPONENT(kb_config.matrix_scaling_params[row][col].b_decimal);
                    for (int adc_val = 0; adc_val < MAX_ADC_READING; adc_val++) {
                        float val = 10.0 * (cbrt(a/(float)adc_val) - b);
                        float fractional_val = (val - (float)(int)val);
                        int sensor = sensor_num[row][col];

                        sensor_lookup_table[sensor_num[row][col]][adc_val] = val; 

                        if (val < 0) {
                            sensor_lookup_table[sensor][adc_val] = X_MIN * 10;
                        } else if (val > X_MAX * 10) {
                            sensor_lookup_table[sensor][adc_val] = X_MAX * 10;
                        } else {
                            // round int up or down
                            if (fractional_val >= 0.5) {
                                sensor_lookup_table[sensor][adc_val] = (int)val + 1;
                            } else {
                                sensor_lookup_table[sensor][adc_val] = (int)val;
                            }
                        } 
                    }
                }
            }
        }
    }
}


void keyboard_post_init_kb(void) {
    debug_enable = true;
    eeconfig_read_kb_datablock(&kb_config);
    create_lookup_table();
}

bool process_record_kb(uint16_t keycode, keyrecord_t* record) {
    if (!process_record_user(keycode, record)) {
        return false;
    }

    switch (keycode) {
    case KC_CALIBRATE:
        if (record->event.pressed) {
            // this will disable analog keys while calibrating
            calibrating_sensors = true;
            for (int row = 0; row < MATRIX_ROWS; row++) {
                for (int col = 0; col < MATRIX_COLS; col++) {
                    if (pin_scan_modes[row][col] == ANALOG) {
                        kb_config.matrix_sensor_bounds[row][col].min = -1;
                        kb_config.matrix_sensor_bounds[row][col].max = 0;
                    }
                }
            }
        } else {
            // runs once after calibration button is released
            calibrating_sensors = false;
            compute_sensor_scaling_params();
            create_lookup_table();
            eeconfig_update_kb_datablock(&kb_config);
        }
        return false;
    case KC_TOGGLE_RAPID_TRIGGER:
        if (record->event.pressed) {
            kb_config.rapid_trigger = !kb_config.rapid_trigger;
            eeconfig_update_kb_datablock(&kb_config);
        }
        return false;
    case KC_ACTUATION_DEC:
        if (kb_config.actuation_point_mm > 1) {
            --kb_config.actuation_point_mm;
            eeconfig_update_kb_datablock(&kb_config);
        }
        return false;
    case KC_ACTUATION_INC:
        if (kb_config.actuation_point_mm < 40) {
            ++kb_config.actuation_point_mm;
            eeconfig_update_kb_datablock(&kb_config);
        }
        return false;
    }

    return true;
}

void matrix_scan_kb(void) {
    static uint16_t key_timer;
    if (timer_elapsed(key_timer) > 1000) {
        key_timer = timer_read();
        dprintf("(%i, %i) (%i, %i) (%i, %i)\n", min1, max1, min2, max2, min3, max3);
        min1 = -1;
        max1 = 0;
        min2 = -1;
        max2 = 0;
        min3 = -1;
        max3 = 0;
    }

    if (calibrating_sensors) {
        for (int row = 0; row < MATRIX_ROWS; row++) {
            for (int col = 0; col < MATRIX_COLS; col++) {
                if (pin_scan_modes[row][col] == ANALOG) {
                    pin_t pin = direct_pins[row][col];
                    uint16_t sensor_value = MAX_ADC_READING - analogReadPin(pin);
                    sensor_bounds_t* bounds = &kb_config.matrix_sensor_bounds[row][col];
                    if (sensor_value < bounds->min) {
                        bounds->min = sensor_value;
                    }
                    if (sensor_value > bounds->max) {
                        bounds->max = sensor_value;
                    }
                }
            }
        }
    }
}

#ifdef VIA_ENABLE
enum via_kb_config_value {
    id_kb_rapid_trigger = 1,
    id_kb_actuation_point_mm = 2,
    id_kb_release_point_mm = 3,
    id_kb_rapid_trigger_sensitivity_mm = 4
};

void kb_config_set_value(uint8_t* data) {
    uint8_t* value_id = &(data[0]);
    uint8_t* value_data = &(data[1]);

    switch (*value_id) {
    case id_kb_rapid_trigger:
        kb_config.rapid_trigger = *value_data;
        break;
    case id_kb_actuation_point_mm:
        kb_config.actuation_point_mm = *value_data;
        break;
    case id_kb_release_point_mm:
        kb_config.release_point_mm = *value_data;
        break;
    case id_kb_rapid_trigger_sensitivity_mm:
        kb_config.rapid_trigger_sensitivity_mm = *value_data;
        break;
    }
}

void kb_config_get_value(uint8_t* data) {
    // data = [ value_id, value_data ]
    uint8_t* value_id = &(data[0]);
    uint8_t* value_data = &(data[1]);

    switch (*value_id) {
    case id_kb_rapid_trigger:
        *value_data = kb_config.rapid_trigger;
        break;
    case id_kb_actuation_point_mm:
        *value_data = kb_config.actuation_point_mm;
        break;
    case id_kb_release_point_mm:
        *value_data = kb_config.release_point_mm;
        break;
    case id_kb_rapid_trigger_sensitivity_mm:
        *value_data = kb_config.rapid_trigger_sensitivity_mm;
        break;
    }
}

void kb_config_save(void) {
    eeconfig_update_kb_datablock(&kb_config);
}

void via_custom_value_command_kb(uint8_t* data, uint8_t length) {
    // data = [ command_id, channel_id, value_id, value_data ]
    uint8_t* command_id = &(data[0]);
    uint8_t* channel_id = &(data[1]);
    uint8_t* value_id_and_data = &(data[2]);

    if (*channel_id == id_custom_channel) {
        switch (*command_id) {
        case id_custom_set_value:
            kb_config_set_value(value_id_and_data);
            break;
        case id_custom_get_value:
            kb_config_get_value(value_id_and_data);
            break;
        case id_custom_save:
            kb_config_save();
            break;
        default:
            // Unhandled message.
            *command_id = id_unhandled;
            break;
        }
        return;
    }

    // Return the unhandled state
    *command_id = id_unhandled;
}
#endif // VIA_ENABLE
