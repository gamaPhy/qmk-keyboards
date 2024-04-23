// Copyright 2023 gamaPhy (https://gamaphy.com/)
// SPDX-License-Identifier: GPL-3.0-or-later

#include <limits.h>
#include <math.h>

#include "freedom.h"
#include "helpers/lookup_table.h"
#include "helpers/sensor_read.h"

#ifdef RGB_MATRIX_ENABLE
led_config_t g_led_config = {{
                                 {12, 13, 14},
                                 {NO_LED, NO_LED, NO_LED},
                             },
                             {
                                 // Underglow
                                 {112, 15},
                                 {61, 12},
                                 {17, 22},
                                 {12, 38},
                                 {37, 44},
                                 {80, 57},
                                 {112, 43},
                                 {143, 57},
                                 {186, 44},
                                 {211, 38},
                                 {206, 22},
                                 {169, 11},
                                 // Key matrix
                                 // TODO: swap these
                                 {112, 27},
                                 {46, 27},
                                 {177, 27},
                             },
                             {
                                 // Underglow
                                 2,
                                 2,
                                 2,
                                 2,
                                 2,
                                 2,
                                 2,
                                 2,
                                 2,
                                 2,
                                 2,
                                 2,
                                 // Key matrix
                                 4,
                                 4,
                                 4,
                             }};
#endif

kb_config_t kb_config;
sensor_bounds_t running_sensor_bounds[SENSOR_COUNT];
uint8_t sensor_lookup_table[SENSOR_COUNT][MAX_ADC_READING];

const pin_t direct_pins[MATRIX_ROWS][MATRIX_COLS] = DIRECT_PINS;
const pin_scan_mode_t pin_scan_modes[MATRIX_ROWS][MATRIX_COLS] = PIN_SCAN_MODES;
const int sensor_nums[MATRIX_ROWS][MATRIX_COLS] = SENSOR_NUMS;

bool bootup_calibrated = false;
uint8_t startup_count = 0;

bool calibrating_sensors = false;

// Our bootmagic implementation allows optionally clearing EEPROM depending on
// whether the BOOTMAGIC_CLEAR button is held down along with the original
// BOOTMAGIC_LITE button. If the EEPROM gets so corrupt that the MCU doesn't get
// to this point, it can be reset manually. For rp2040, this requires loading
// `flash_nuke.uf2` which can be found in our repo, or at:
// https://www.raspberrypi.com/documentation/microcontrollers/raspberry-pi-pico.html#resetting-flash-memory
void bootmagic_lite(void) {
  matrix_scan();
  wait_ms(30);
  matrix_scan();

  if (matrix_get_row(BOOTMAGIC_LITE_ROW) & (1 << BOOTMAGIC_LITE_COLUMN)) {
    if (matrix_get_row(BOOTMAGIC_CLEAR_ROW) & (1 << BOOTMAGIC_CLEAR_COLUMN)) {
      eeconfig_disable();
    }
    bootloader_jump();
  }
}

void kb_config_save(void) { eeconfig_update_kb_datablock(&kb_config); }

void eeconfig_init_kb(void) {
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
  kb_config_save();
}

bool calibration_successful(void) {
  for (int row = 0; row < MATRIX_ROWS; row++) {
    for (int col = 0; col < MATRIX_COLS; col++) {
      if (pin_scan_modes[row][col] == ANALOG) {
        if (kb_config.matrix_sensor_bounds[row][col].max -
                kb_config.matrix_sensor_bounds[row][col].min <
            MIN_SENSOR_BOUND_RANGE) {
          // the sensors were not pressed down a sufficient amount
          return false;
        }
      }
    }
  }
  return true;
}

void keyboard_pre_init_user(void) {
  setPinOutput(PICO_LED);
  setPinOutput(WS2812_DI_PIN);
}

void keyboard_post_init_user(void) {
  rgb_matrix_sethsv_noeeprom(HSV_BLACK);
  debug_enable = true;
  // have to turn on the rgb again after s min values have been calibrated
  eeconfig_read_kb_datablock(&kb_config);
  for (int s = 0; s < SENSOR_COUNT; s++) {
    running_sensor_bounds[s].min = -1;
    running_sensor_bounds[s].max = 0;
  }
  // need a dummy adc_read before enabling temperature sensor
  // https://github.com/qmk/qmk_firmware/pull/19453#issuecomment-1383271354
  adc_read(TO_MUX(4, 0));
  adcRPEnableTS(&ADCD1);
}

bool process_record_kb(uint16_t keycode, keyrecord_t *record) {
  if (!process_record_user(keycode, record)) {
    return false;
  }

  switch (keycode) {
  case KC_CALIBRATE:
    if (record->event.pressed) {
      rgb_matrix_sethsv_noeeprom(HSV_BLACK);
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

      if (calibration_successful()) {
        rgb_matrix_reload_from_eeprom();
        kb_config.calibrated = true;
        create_lookup_table(&kb_config, sensor_lookup_table);
        kb_config_save();
      } else {
        if (kb_config.calibrated) {
          // return to state before calibration started
          rgb_matrix_reload_from_eeprom();
          eeconfig_read_kb_datablock(&kb_config);
        }
      }
      writePinLow(PICO_LED);
    }
    return false;
  case KC_TOGGLE_RAPID_TRIGGER:
    if (record->event.pressed) {
      kb_config.global_actuation_settings.rapid_trigger =
          !kb_config.global_actuation_settings.rapid_trigger;
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

    dprintf("\nTemperature: %i\n", adc_read(TO_MUX(4, 0)));
    dprintf("\nCalibrated range:\n(%i, %i) (%i, %i) (%i, %i)\n\n",
            kb_config.matrix_sensor_bounds[0][0].min,
            kb_config.matrix_sensor_bounds[0][0].max,
            kb_config.matrix_sensor_bounds[0][1].min,
            kb_config.matrix_sensor_bounds[0][1].max,
            kb_config.matrix_sensor_bounds[0][2].min,
            kb_config.matrix_sensor_bounds[0][2].max);

    if (!bootup_calibrated) {
      startup_count++;
      // wait a bit for stable equilibrium, otherwise sensor min values will be
      // off
      if (startup_count == 2) {
        for (int s = 0; s < SENSOR_COUNT; s++) {
          kb_config.matrix_sensor_bounds[0][s].min =
              running_sensor_bounds[s].min;
        }
        bootup_calibrated = true;
        create_lookup_table(&kb_config, sensor_lookup_table);
        rgb_matrix_reload_from_eeprom();
      }
    } else {
      dprintf("Current reading range within 1 second:\n");
      for (int s = 0; s < SENSOR_COUNT; s++) {
        dprintf("(%i,%i) ", running_sensor_bounds[s].min,
                running_sensor_bounds[s].max);
      }

      dprintf("Key press distance (0 - 80):\n ");
      for (int s = 0; s < SENSOR_COUNT; s++) {
        dprintf("(%i, %i) ",
                sensor_lookup_table[s][running_sensor_bounds[s].min],
                sensor_lookup_table[s][running_sensor_bounds[s].max]);
      }
      dprintf("\n\n");

      for (int s = 0; s < SENSOR_COUNT; s++) {
        running_sensor_bounds[s].min = -1;
        running_sensor_bounds[s].max = 0;
      }
    }

    dprintf(
        "\n################################################################\n");
  }

  if (calibrating_sensors) {
    for (int row = 0; row < MATRIX_ROWS; row++) {
      for (int col = 0; col < MATRIX_COLS; col++) {
        if (pin_scan_modes[row][col] == ANALOG) {
          pin_t pin = direct_pins[row][col];
          uint16_t sensor_value = oversample(pin);
          sensor_bounds_t *bounds = &kb_config.matrix_sensor_bounds[row][col];
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
  id_kb_global_actuation_settings_rapid_trigger_release_sensitivity_dmm,
  id_kb_per_key_left_rapid_trigger,
  id_kb_per_key_actuation_settings_left_actuation_distance,
  id_kb_per_key_actuation_settings_left_press_sensitivity,
  id_kb_per_key_actuation_settings_left_release_sensitivity,
  id_kb_per_key_middle_rapid_trigger,
  id_kb_per_key_actuation_settings_middle_actuation_distance,
  id_kb_per_key_actuation_settings_middle_press_sensitivity,
  id_kb_per_key_actuation_settings_middle_release_sensitivity,
  id_kb_per_key_right_rapid_trigger,
  id_kb_per_key_actuation_settings_right_actuation_distance,
  id_kb_per_key_actuation_settings_right_press_sensitivity,
  id_kb_per_key_actuation_settings_right_release_sensitivity
};

void kb_config_set_value(uint8_t *data) {
  uint8_t *value_id = &(data[0]);
  uint8_t *value_data = &(data[1]);

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
    kb_config.global_actuation_settings.rapid_trigger_press_sensitivity_dmm =
        *value_data;
    break;
  case id_kb_global_actuation_settings_rapid_trigger_release_sensitivity_dmm:
    kb_config.global_actuation_settings.rapid_trigger_release_sensitivity_dmm =
        *value_data;
    break;
  case id_kb_per_key_left_rapid_trigger:
    kb_config.per_key_actuation_settings[0].rapid_trigger = *value_data;
    break;
  case id_kb_per_key_actuation_settings_left_actuation_distance:
    kb_config.per_key_actuation_settings[0].actuation_point_dmm = *value_data;
    break;
  case id_kb_per_key_actuation_settings_left_press_sensitivity:
    kb_config.per_key_actuation_settings[0]
        .rapid_trigger_press_sensitivity_dmm = *value_data;
    break;
  case id_kb_per_key_actuation_settings_left_release_sensitivity:
    kb_config.per_key_actuation_settings[0]
        .rapid_trigger_release_sensitivity_dmm = *value_data;
    break;
  case id_kb_per_key_middle_rapid_trigger:
    kb_config.per_key_actuation_settings[1].rapid_trigger = *value_data;
    break;
  case id_kb_per_key_actuation_settings_middle_actuation_distance:
    kb_config.per_key_actuation_settings[1].actuation_point_dmm = *value_data;
    break;
  case id_kb_per_key_actuation_settings_middle_press_sensitivity:
    kb_config.per_key_actuation_settings[1]
        .rapid_trigger_press_sensitivity_dmm = *value_data;
    break;
  case id_kb_per_key_actuation_settings_middle_release_sensitivity:
    kb_config.per_key_actuation_settings[1]
        .rapid_trigger_release_sensitivity_dmm = *value_data;
    break;
  case id_kb_per_key_right_rapid_trigger:
    kb_config.per_key_actuation_settings[2].rapid_trigger = *value_data;
    break;
  case id_kb_per_key_actuation_settings_right_actuation_distance:
    kb_config.per_key_actuation_settings[2].actuation_point_dmm = *value_data;
    break;
  case id_kb_per_key_actuation_settings_right_press_sensitivity:
    kb_config.per_key_actuation_settings[2]
        .rapid_trigger_press_sensitivity_dmm = *value_data;
    break;
  case id_kb_per_key_actuation_settings_right_release_sensitivity:
    kb_config.per_key_actuation_settings[2]
        .rapid_trigger_release_sensitivity_dmm = *value_data;
    break;
  }
}

void kb_config_get_value(uint8_t *data) {
  // data = [ value_id, value_data ]
  uint8_t *value_id = &(data[0]);
  uint8_t *value_data = &(data[1]);

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
    *value_data =
        kb_config.global_actuation_settings.rapid_trigger_press_sensitivity_dmm;
    break;
  case id_kb_global_actuation_settings_rapid_trigger_release_sensitivity_dmm:
    *value_data = kb_config.global_actuation_settings
                      .rapid_trigger_release_sensitivity_dmm;
    break;
  case id_kb_per_key_left_rapid_trigger:
    *value_data = kb_config.per_key_actuation_settings[0].rapid_trigger;
    break;
  case id_kb_per_key_actuation_settings_left_actuation_distance:
    *value_data = kb_config.per_key_actuation_settings[0].actuation_point_dmm;
    break;
  case id_kb_per_key_actuation_settings_left_press_sensitivity:
    *value_data = kb_config.per_key_actuation_settings[0]
                      .rapid_trigger_press_sensitivity_dmm;
    break;
  case id_kb_per_key_actuation_settings_left_release_sensitivity:
    *value_data = kb_config.per_key_actuation_settings[0]
                      .rapid_trigger_release_sensitivity_dmm;
    break;
  case id_kb_per_key_middle_rapid_trigger:
    *value_data = kb_config.per_key_actuation_settings[1].rapid_trigger;
    break;
  case id_kb_per_key_actuation_settings_middle_actuation_distance:
    *value_data = kb_config.per_key_actuation_settings[1].actuation_point_dmm;
    break;
  case id_kb_per_key_actuation_settings_middle_press_sensitivity:
    *value_data = kb_config.per_key_actuation_settings[1]
                      .rapid_trigger_press_sensitivity_dmm;
    break;
  case id_kb_per_key_actuation_settings_middle_release_sensitivity:
    *value_data = kb_config.per_key_actuation_settings[1]
                      .rapid_trigger_release_sensitivity_dmm;
    break;
  case id_kb_per_key_right_rapid_trigger:
    *value_data = kb_config.per_key_actuation_settings[2].rapid_trigger;
    break;
  case id_kb_per_key_actuation_settings_right_actuation_distance:
    *value_data = kb_config.per_key_actuation_settings[2].actuation_point_dmm;
    break;
  case id_kb_per_key_actuation_settings_right_press_sensitivity:
    *value_data = kb_config.per_key_actuation_settings[2]
                      .rapid_trigger_press_sensitivity_dmm;
    break;
  case id_kb_per_key_actuation_settings_right_release_sensitivity:
    *value_data = kb_config.per_key_actuation_settings[2]
                      .rapid_trigger_release_sensitivity_dmm;
    break;
  }
}

void via_custom_value_command_kb(uint8_t *data, uint8_t length) {
  // data = [ command_id, channel_id, value_id, value_data ]
  uint8_t *command_id = &(data[0]);
  uint8_t *channel_id = &(data[1]);
  uint8_t *value_id_and_data = &(data[2]);

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
