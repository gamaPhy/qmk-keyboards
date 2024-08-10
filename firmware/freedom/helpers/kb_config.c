#include "quantum.h"

#include "kb_config.h"

kb_config_t kb_config;
kb_config_t stored_kb_config;

void kb_config_save_to_eeprom(void) {
  eeconfig_update_kb_datablock(&kb_config);
  memcpy(&stored_kb_config, &kb_config, sizeof(kb_config));
}

void kb_config_reload_from_eeprom(void) {
  eeconfig_read_kb_datablock(&kb_config);
  memcpy(&stored_kb_config, &kb_config, sizeof(kb_config));
}

void kb_config_factory_reset(void) {
  kb_config.calibrated = false;
  kb_config.use_per_key_settings = false;
  kb_config.global_actuation_settings.rapid_trigger = true;
  kb_config.global_actuation_settings.actuation_point_dmm = 6;
  kb_config.global_actuation_settings.rapid_trigger_press_sensitivity_dmm = 2;
  kb_config.global_actuation_settings.rapid_trigger_release_sensitivity_dmm = 4;
  for (int i = 0; i < SENSOR_COUNT; i++) {
    kb_config.per_key_actuation_settings[i].rapid_trigger = true;
    kb_config.per_key_actuation_settings[i].actuation_point_dmm = 6;
    kb_config.per_key_actuation_settings[i]
        .rapid_trigger_press_sensitivity_dmm = 2;
    kb_config.per_key_actuation_settings[i]
        .rapid_trigger_release_sensitivity_dmm = 4;
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
}
