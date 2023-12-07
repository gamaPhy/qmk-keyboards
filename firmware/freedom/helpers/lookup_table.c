#include "lookup_table.h"

#include <math.h>
#include <limits.h>

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
int determine_sensor_base_offset(kb_config_t *kb_config) {
    int min_total = 0;
    int count = 0;
    for (int row = 0; row < MATRIX_ROWS; row++) {
        for (int col = 0; col < MATRIX_COLS; col++) {
            if (pin_scan_modes[row][col] == ANALOG) {
                min_total += kb_config->matrix_sensor_bounds[row][col].min;
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


int mm_to_lookup_table_val(float val) {
    return val * LOOKUP_TABLE_MULTIPLIER * 10.0;
}

// Computes and stores the `a` and `b` parameters of the best-fit scaling equation. 
void compute_sensor_scaling_params(kb_config_t *kb_config){
    const int sensor_base_offset = determine_sensor_base_offset(kb_config);
    for (int row = 0; row < MATRIX_ROWS; row++) {
        for (int col = 0; col < MATRIX_COLS; col++) {
            if (pin_scan_modes[row][col] == ANALOG) {
                int max = kb_config->matrix_sensor_bounds[row][col].max;
                int min = kb_config->matrix_sensor_bounds[row][col].min;
                kb_config->matrix_scaling_params[row][col].base_value = min - sensor_base_offset; 
                int base_val = kb_config->matrix_scaling_params[row][col].base_value;

                float b_param = compute_b_param(max, min, base_val);
                kb_config->matrix_scaling_params[row][col].b = b_param;
                kb_config->matrix_scaling_params[row][col].b_fractional_component = fractional_component_as_int(b_param);
                kb_config->matrix_scaling_params[row][col].a = compute_a_param(b_param, max, min, base_val);

                dprintf("Sensor MIN: %i\n", (int) min);
                dprintf("Sensor MAX: %i\n", (int) max);
                dprintf("A: %li\n", kb_config->matrix_scaling_params[row][col].a);
                dprintf("B: %i\n", kb_config->matrix_scaling_params[row][col].b);
                dprintf("B decimal: %li / %i\n", kb_config->matrix_scaling_params[row][col].b_fractional_component, INT_MAX);
                dprintf("BASE: %i\n", kb_config->matrix_scaling_params[row][col].base_value);
            }
        }
    }
}

void create_lookup_table(kb_config_t *kb_config, uint8_t sensor_lookup_table[SENSOR_COUNT][MAX_ADC_READING]) {
    compute_sensor_scaling_params(kb_config);
    
    if(sensor_lookup_table != NULL) {
        for (int row = 0; row < MATRIX_ROWS; row++) {
            for (int col = 0; col < MATRIX_COLS; col++) {
                if (pin_scan_modes[row][col] == ANALOG) {
                    float a = (float)kb_config->matrix_scaling_params[row][col].a;
                    float b = int_components_to_float(kb_config->matrix_scaling_params[row][col].b, kb_config->matrix_scaling_params[row][col].b_fractional_component);
                    float base = (float)kb_config->matrix_scaling_params[row][col].base_value;
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