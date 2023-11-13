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
const int sensor_nums[MATRIX_ROWS][MATRIX_COLS] = SENSOR_NUMS;

extern uint8_t (*sensor_lookup_table)[MAX_ADC_READING];

uint16_t min1, max1, min2, max2, min3, max3;

// Our bootmagic implementation does not clear EEPROM,
// primarily so that hall effect sensor calibration does not get cleared.
// This means that if you want to clear EEPROM, it must be done manually.
// For rp2040, this requires loading `flash_nuke.uf2` which can be found in our directory, or at:
// https://www.raspberrypi.com/documentation/microcontrollers/raspberry-pi-pico.html#resetting-flash-memory
void bootmagic_lite(void) {
    matrix_scan();
    wait_ms(30);
    matrix_scan();

    if (matrix_get_row(BOOTMAGIC_LITE_ROW) & (1 << BOOTMAGIC_LITE_COLUMN)) {
      // Jump to bootloader.
      bootloader_jump();
    }
}

void kb_config_save(void) {
    eeconfig_update_kb_datablock(&kb_config);
}

void eeconfig_init_kb(void) {
    kb_config.calibrated = false;
    kb_config.use_per_key_settings = false;
    kb_config.global_actuation_settings.rapid_trigger = true;
    kb_config.global_actuation_settings.actuation_point_dmm = 6;
    kb_config.global_actuation_settings.rapid_trigger_press_sensitivity_dmm = 2;
    kb_config.global_actuation_settings.rapid_trigger_release_sensitivity_dmm = 5;
    for (int i = 0; i < SENSOR_COUNT; i++) {
        kb_config.per_key_actuation_settings[i].rapid_trigger = true;
        kb_config.per_key_actuation_settings[i].actuation_point_dmm = 6;
        kb_config.per_key_actuation_settings[i].rapid_trigger_press_sensitivity_dmm = 2;
        kb_config.per_key_actuation_settings[i].rapid_trigger_release_sensitivity_dmm = 5;
    }
    for (int row = 0; row < MATRIX_ROWS; row++) {
        for (int col = 0; col < MATRIX_COLS; col++) {
            if (pin_scan_modes[row][col] == ANALOG) {
                kb_config.matrix_sensor_bounds[row][col].min = -1;
                kb_config.matrix_sensor_bounds[row][col].max = 0;
                kb_config.matrix_scaling_params[row][col].a = 0;
                kb_config.matrix_scaling_params[row][col].b = 0;
                kb_config.matrix_scaling_params[row][col].b_fractional_component = 0;
                kb_config.matrix_scaling_params[row][col].base_value = 0;
            }
        }
    }
    kb_config_save();
}

float ratio(int sensor_max, int sensor_min, int base_val){
    return ((float)sensor_max - (float)base_val)/((float)sensor_min - (float)base_val);
}

float compute_b_param(int sensor_max, int sensor_min, int base_val) {
    return (float)((X_MAX_mm * cbrt(ratio(sensor_max, sensor_min, base_val)) - X_MIN_mm) / (1.0 - cbrt(ratio(sensor_max, sensor_min, base_val))));
}

float compute_a_param(float b_param, int sensor_max, int sensor_min, int base_val) {
    return ((float)sensor_min - (float)base_val) * (float)pow(b_param + X_MIN_mm, 3);
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

// Guesses what board is in use and returns the corresponding base offset value
// Hardcoded for SLSS49E sensors
int determine_sensor_base_offset(void) {
    int min_total = 0;
    int count = 0;
    for (int row = 0; row < MATRIX_ROWS; row++) {
        for (int col = 0; col < MATRIX_COLS; col++) {
            if (pin_scan_modes[row][col] == ANALOG) {
                min_total += kb_config.matrix_sensor_bounds[row][col].min;
                count++;
            }
        }
    }

    const int min_avg = min_total / count;
    if (min_avg > 1500) {
        return PICO_SENSOR_BASE_OFFSET;
    } 
    return SENSOR_BASE_OFFSET_4V4_2V5;
}

// Computes and stores the `a` and `b` parameters of the best-fit scaling equation. 
void compute_sensor_scaling_params(void){
    const int sensor_base_offset = determine_sensor_base_offset();
    for (int row = 0; row < MATRIX_ROWS; row++) {
        for (int col = 0; col < MATRIX_COLS; col++) {
            if (pin_scan_modes[row][col] == ANALOG) {
                int max = kb_config.matrix_sensor_bounds[row][col].max;
                int min = kb_config.matrix_sensor_bounds[row][col].min;
                kb_config.matrix_scaling_params[row][col].base_value = min - sensor_base_offset; 
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

int mm_to_lookup_table_val(float val) {
    return val * LOOKUP_TABLE_MULTIPLIER * 10.0;
}

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
                        float val_mm = (cbrt(a/((float)adc_val - base)) - b);
                        int sensor = sensor_nums[row][col];

                        if (val_mm < X_MIN_mm) {
                            sensor_lookup_table[sensor][adc_val] = mm_to_lookup_table_val(X_MIN_mm);
                        } else if (mm_to_lookup_table_val(val_mm) > KEY_MAX_dmm * LOOKUP_TABLE_MULTIPLIER) {
                            sensor_lookup_table[sensor][adc_val] = KEY_MAX_dmm * LOOKUP_TABLE_MULTIPLIER;
                        } else {
                            sensor_lookup_table[sensor][adc_val] = mm_to_lookup_table_val(val_mm);
                        } 
                    }
                }
            }
        }
    }
}

bool calibration_successful(void) {
    for (int row = 0; row < MATRIX_ROWS; row++) {
        for (int col = 0; col < MATRIX_COLS; col++) {
            if (pin_scan_modes[row][col] == ANALOG) {
                if (kb_config.matrix_sensor_bounds[row][col].max - kb_config.matrix_sensor_bounds[row][col].min < MIN_SENSOR_BOUND_RANGE) {
                    // the sensors were not pressed down a sufficient amount
                    return false;
                }
            }
        }
    }
    return true;
}

void keyboard_post_init_user(void) {
    debug_enable = true;
    setPinOutput(PICO_LED);
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
            rgblight_sethsv_noeeprom(HSV_BLACK);
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
            calibrating_sensors = false;

            if(calibration_successful()) {
                rgblight_reload_from_eeprom();
                writePinLow(PICO_LED);
                kb_config.calibrated = true;
                compute_sensor_scaling_params();
                create_lookup_table();

                kb_config_save();
            } else {
                if(kb_config.calibrated) {
                    //return to state before calibration started
                    rgblight_reload_from_eeprom();
                    writePinLow(PICO_LED);
                }
            }
        }
        return false;
    case KC_TOGGLE_RAPID_TRIGGER:
        if (record->event.pressed) {
            kb_config.global_actuation_settings.rapid_trigger = !kb_config.global_actuation_settings.rapid_trigger;
            kb_config_save();
        }
        return false;
    case KC_ACTUATION_DEC:
        if (kb_config.global_actuation_settings.actuation_point_dmm > 1) {
            --kb_config.global_actuation_settings.actuation_point_dmm;
            kb_config_save();
        }
        return false;
    case KC_ACTUATION_INC:
        if (kb_config.global_actuation_settings.actuation_point_dmm < 40) {
            ++kb_config.global_actuation_settings.actuation_point_dmm;
            kb_config_save();
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
        dprintf("(%i, %i) (%i, %i) (%i, %i)\n\n", 
                sensor_lookup_table[0][min1], sensor_lookup_table[0][max1], 
                sensor_lookup_table[1][min2], sensor_lookup_table[1][max2], 
                sensor_lookup_table[2][min3], sensor_lookup_table[2][max3]);
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
                    uint16_t sensor_value = oversample(pin);
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
    id_kb_use_per_key_settings = 1,
    id_kb_global_actuation_settings_rapid_trigger,
    id_kb_global_actuation_settings_actuation_point_dmm,
    id_kb_global_actuation_settings_rapid_trigger_press_sensitivity_dmm,
    id_kb_global_actuation_settings_rapid_trigger_release_sensitivity_dmm
};

void kb_config_set_value(uint8_t* data) {
    uint8_t* value_id = &(data[0]);
    uint8_t* value_data = &(data[1]);

    switch (*value_id) {
    case id_kb_use_per_key_settings:
        kb_config.use_per_key_settings = *value_data;
        break;
    case id_kb_global_actuation_settings_rapid_trigger:
        kb_config.global_actuation_settings.rapid_trigger = *value_data;
        break;
    case id_kb_global_actuation_settings_actuation_point_dmm:
        kb_config.global_actuation_settings.actuation_point_dmm = *value_data;
        break;
    case id_kb_global_actuation_settings_rapid_trigger_press_sensitivity_dmm:
        kb_config.global_actuation_settings.rapid_trigger_press_sensitivity_dmm = *value_data;
        break;
    case id_kb_global_actuation_settings_rapid_trigger_release_sensitivity_dmm:
        kb_config.global_actuation_settings.rapid_trigger_release_sensitivity_dmm = *value_data;
        break;
    }
}

void kb_config_get_value(uint8_t* data) {
    // data = [ value_id, value_data ]
    uint8_t* value_id = &(data[0]);
    uint8_t* value_data = &(data[1]);

    switch (*value_id) {
    case id_kb_use_per_key_settings:
        *value_data = kb_config.use_per_key_settings;
        break;
    case id_kb_global_actuation_settings_rapid_trigger:
        *value_data = kb_config.global_actuation_settings.rapid_trigger;
        break;
    case id_kb_global_actuation_settings_actuation_point_dmm:
        *value_data = kb_config.global_actuation_settings.actuation_point_dmm;
        break;
    case id_kb_global_actuation_settings_rapid_trigger_press_sensitivity_dmm:
        *value_data = kb_config.global_actuation_settings.rapid_trigger_press_sensitivity_dmm;
        break;
    case id_kb_global_actuation_settings_rapid_trigger_release_sensitivity_dmm:
        *value_data = kb_config.global_actuation_settings.rapid_trigger_release_sensitivity_dmm;
        break;
    }
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
