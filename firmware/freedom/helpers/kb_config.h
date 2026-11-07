#pragma once

typedef enum { NO_SCAN, DIGITAL, ANALOG } pin_scan_mode_t;

typedef struct {
  uint16_t min;
  uint16_t max;
} sensor_bounds_t;

// Parameters of sensor scaling equation:
// y(x) = (a/(x+b)^3) + base_value
// where `base_value` is the value output from a HE sensor with no switches
// placed on the PCB.
typedef struct {
  int32_t a;
  int16_t b;
  // QMK can't store `float`s in EEPROM, so fractional values have to be
  // decomposed into `int`s, then reconstructed upon use The actual decimal
  // value of `b` is derived by doing `(float)b_fractional_component /
  // (float)INT_MAX`
  int32_t b_fractional_component;
  uint16_t base_value;
} sensor_scaling_params_t;

typedef struct {
  bool rapid_trigger;
  uint8_t actuation_point_dmm;
  uint8_t rapid_trigger_press_sensitivity_dmm;
  uint8_t rapid_trigger_release_sensitivity_dmm;
} actuation_settings_t;

typedef struct {
  bool calibrated;
  bool use_per_key_settings;
  actuation_settings_t global_actuation_settings;
  actuation_settings_t per_key_actuation_settings[SENSOR_COUNT];
  sensor_bounds_t matrix_sensor_bounds[MATRIX_ROWS][MATRIX_COLS];
  sensor_scaling_params_t matrix_scaling_params[MATRIX_ROWS][MATRIX_COLS];
} kb_config_t;

void kb_config_save_to_eeprom(void);
void kb_config_reload_from_eeprom(void);
void kb_config_factory_reset(void);
int clamp_setpoint_dmm(int);

extern kb_config_t kb_config;
// reflects kb_config in non-volatile memory
extern kb_config_t stored_kb_config;
extern const pin_scan_mode_t pin_scan_modes[MATRIX_ROWS][MATRIX_COLS];
extern const int sensor_nums[MATRIX_ROWS][MATRIX_COLS];
