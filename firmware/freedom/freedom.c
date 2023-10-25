// Copyright 2023 gamaPhy (https://gamaphy.com/)
// SPDX-License-Identifier: GPL-3.0-or-later

#include <math.h>
#include <limits.h>

#include "freedom.h"
#include "sensor_read.h"

bool calibrating_sensors = false;

kb_config_t kb_config;

const pin_t direct_pins[MATRIX_ROWS][MATRIX_COLS] = DIRECT_PINS;
const pin_scan_mode_t pin_scan_modes[MATRIX_ROWS][MATRIX_COLS] = PIN_SCAN_MODES;
const int sensor_num[MATRIX_ROWS][MATRIX_COLS] = SENSOR_NUM;

// There are minimum of 40 (4mm/0.1mm) distances that need to be represented 
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
                kb_config.matrix_scaling_params[row][col].b_fractional_component = 0;
                kb_config.matrix_scaling_params[row][col].base_value = 0;
            }
        }
    }
    eeconfig_update_kb_datablock(&kb_config);
}

// A sensor at X_MIN is when the switch is completely released. A sensor at X_MAX is completely pressed.
#define X_MIN (float)0
// The greatest displacement in mm that a magnetic switch can be pressed according to Gateron datasheet
#define X_MAX (float)4.1

// sensor_max is assumed to be where the key is at X_MAX, ie. completely pressed.
// sensor_min is assumed to be where the key is at X_MIN, ie. completely released.
float ratio(int sensor_max, int sensor_min, int base_val){
    return ((float)sensor_max - (float)base_val)/((float)sensor_min - (float)base_val);
}

float compute_b_param(int sensor_max, int sensor_min, int base_val) {
    return (float)((X_MAX * cbrt(ratio(sensor_max, sensor_min, base_val)) - X_MIN) / (1.0 - cbrt(ratio(sensor_max, sensor_min, base_val))));
}

float compute_a_param(float b_param, int sensor_max, int sensor_min, int base_val) {
    return ((float)sensor_min - (float)base_val) * (pow(b_param, 3) + X_MIN);
}

// transforms the fractional component of a value to an int 
// Ex: The fractional component of 9.123 is 0.123 
int fractional_component_as_int(float val){
    return (val - (float)(int)val) * (float)INT_MAX;
}

float int_components_to_float(int val, int fractional_component_as_int){
    float fractional_component_as_float = (float)fractional_component_as_int / (float)INT_MAX;
    return val + fractional_component_as_float;
}

// Computes and stores the `a` and `b` parameters of the best-fit scaling equation. 
void compute_sensor_scaling_params(void){
    for (int row = 0; row < MATRIX_ROWS; row++) {
        for (int col = 0; col < MATRIX_COLS; col++) {
            if (pin_scan_modes[row][col] == ANALOG) {
                int min = kb_config.matrix_sensor_bounds[row][col].min;
                int max = kb_config.matrix_sensor_bounds[row][col].max;
                int base_val = kb_config.matrix_scaling_params[row][col].base_value;

                float b_param = compute_b_param(max, min, base_val);
                kb_config.matrix_scaling_params[row][col].b = b_param;
                kb_config.matrix_scaling_params[row][col].b_fractional_component = fractional_component_as_int(b_param);
                kb_config.matrix_scaling_params[row][col].a = compute_a_param(b_param, max, min, base_val);

                dprintf("Sensor MIN: %i\n", (int) min);
                dprintf("Sensor MAX: %i\n", (int) max);
                dprintf("A: %li\n", kb_config.matrix_scaling_params[row][col].a);
                dprintf("B: %i\n", kb_config.matrix_scaling_params[row][col].b);
                dprintf("B decimal: %li / %i\n", kb_config.matrix_scaling_params[row][col].b_fractional_component, INT_MAX);
                dprintf("BASE: %i\n", kb_config.matrix_scaling_params[row][col].base_value);
            }
        }
    }
}

// Calculates and stores each cell of the lookup table, and pads the rest with either X_MIN or X_MAX depending the side of the array
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
                    float b = int_components_to_float(kb_config.matrix_scaling_params[row][col].b, kb_config.matrix_scaling_params[row][col].b_fractional_component);
                    float base = (float)kb_config.matrix_scaling_params[row][col].base_value;
                    for (int adc_val = 0; adc_val < MAX_ADC_READING; adc_val++) {
                        float val = 10.0 * (cbrt(a/((float)adc_val - base)) - b);
                        float fractional_val = (val - (float)(int)val);
                        int sensor = sensor_num[row][col];

                        sensor_lookup_table[sensor_num[row][col]][adc_val] = val; 

                        if (val < X_MIN) {
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
    setPinOutput(PICO_LED);
}

bool process_record_kb(uint16_t keycode, keyrecord_t* record) {
    if (!process_record_user(keycode, record)) {
        return false;
    }

    switch (keycode) {
    case KC_CALIBRATE:
        if (record->event.pressed) {
            rgblight_sethsv_noeeprom(HSV_BLACK);
            // For Pico users, notify of calibration by turning on-board LED on.
            writePinHigh(PICO_LED);

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
            rgblight_reload_from_eeprom();
            writePinLow(PICO_LED);

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
                    uint16_t sensor_value = sensorRead(pin);
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
