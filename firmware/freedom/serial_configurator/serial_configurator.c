// TODO: add new reset on main menu for actuation, lighting, and keymap
// separately or all together
// TODO: notify when the keeb is not calibrated

#include <stdio.h>

#include "quantum.h"
#include "virtser.h"

#include "../helpers/kb_config.h"

#include "../config.h"
#include "serial_configurator.h"
#include "serial_configurator_helpers/lighting_effect.h"
#include "serial_configurator_helpers/setting_bar.h"

#define NL "\n\r"
#define BS 8
#define DEL 127
#define ESC 27

#define ACTUATION_MENU_STR " MAIN MENU -> ACTUATION SETTINGS"

// Denotes which sliding-bar setting is currently selected to be changed
#define SELECT_PREFIX " --> "
#define SELECT_SUFFIX " <--"
#define EMPTY_SUFFIX "    "

#define PER_KEY_SETTINGS_ON_STR " p = Per-Key Settings <ON> "
#define PER_KEY_SETTINGS_OFF_STR " p = Per-Key Settings <OFF>"
#define RAPID_TRIGGER_ON_STR " r = Rapid Trigger    <ON> "
#define RAPID_TRIGGER_OFF_STR " r = Rapid Trigger    <OFF>"

enum Menu {
  MAIN,
  SET_ACTUATION,
  SET_PRESS_SENSITIVITY,
  SET_RELEASE_SENSITIVITY,
  KEYMAP,
  LIGHTING,
  SET_EFFECT,
  SET_SPEED,
  SET_BRIGHTNESS,
  SET_COLOR,
  RESTORE_DEFAULT,
};

kb_config_t kb_config;
kb_config_t stored_kb_config;

two_digit_setting_bar actuation_setting_bar;
two_digit_setting_bar release_setting_bar;
two_digit_setting_bar press_setting_bar;
three_digit_setting_bar speed_setting_bar;
three_digit_setting_bar brightness_setting_bar;
three_digit_setting_bar hue_setting_bar;
three_digit_setting_bar saturation_setting_bar;

bool receiving_cursor_location = false;

typedef struct {
  char row[2];
  char col[2];
} cursor_position_t;

cursor_position_t saved_cursor_positions[2];

int saved_cusor_line[2];
int saved_cusor_column[2];

void send_string_serial(char *str) {
  int i;
  while (str[i] != '\0') {
    virtser_send(str[i]);
    i++;
  }
}

void cursor_up(void) { send_string_serial("\033[1A"); }

void cursor_down(void) { send_string_serial("\033[1B"); }

void cursor_right(void) { send_string_serial("\033[1C"); }

void cursor_left(void) { send_string_serial("\033[1D"); }

void clear_terminal(void) { send_string_serial("\033[2J"); }

void cursor_home(void) { send_string_serial("\033[H"); }

void hide_cursor(void) { send_string_serial("\033[?25h"); }

void request_cursor_location(void) {
  receiving_cursor_location = true;
  send_string_serial("\033[6n");
}

void reset_terminal(void) {
  clear_terminal();
  cursor_home();
}

void print_strings_serial(char *lines[]) {
  int i;
  while (lines[i] != NULL) {
    send_string_serial(lines[i]);
    i++;
  }
}

void print_main_menu(void) {
  char *menu_strings[] = {NL,
                          " MAIN MENU",
                          NL,
                          NL,
                          " a = Actuation",
                          NL,
                          NL,
                          " l = Lighting",
                          NL,
                          NL,
                          " k = Keymap",
                          NL,
                          NL,
                          NULL};
  print_strings_serial(menu_strings);
  cursor_right();
}

void print_per_key_actuation_menu(void) {
  char *menu_strings[] = {NL,
                          " MAIN MENU -> ACTUATION SETTINGS",
                          NL,
                          NL,
                          PER_KEY_SETTINGS_ON_STR,
                          NL,
                          NL,
                          " l = Left Key",
                          NL,
                          NL,
                          " m = Middle Key",
                          NL,
                          NL,
                          " r = Right Key",
                          NL,
                          NL,
                          " b = Back <-",
                          NL,
                          NL,
                          NULL};
  print_strings_serial(menu_strings);
  cursor_right();
}

void print_standard_actuation_menu(void) {
  char *changes = "+";

  if (kb_config.global_actuation_settings.actuation_point_dmm ==
      stored_kb_config.global_actuation_settings.actuation_point_dmm) {
    changes = " ";
  }

  char *menu_strings[] = {NL,
                          " MAIN MENU -> ACTUATION SETTINGS",
                          NL,
                          NL,
                          PER_KEY_SETTINGS_OFF_STR,
                          NL,
                          NL,
                          RAPID_TRIGGER_OFF_STR,
                          NL,
                          NL,
                          NL,
                          SELECT_PREFIX,
                          "Actuation Distance   ",
                          actuation_setting_bar,
                          SELECT_SUFFIX,
                          NL,
                          NL,
                          NL,
                          " d = -1",
                          NL,
                          NL,
                          " I = +5",
                          NL,
                          NL,
                          " D = -5",
                          NL,
                          NL,
                          " c = Clear Changes",
                          NL,
                          NL,
                          " s = Save Settings",
                          changes,
                          NL,
                          NL,
                          " b = Back <-",
                          NL,
                          NL,
                          NULL};
  print_strings_serial(menu_strings);
  cursor_right();
}

void print_rapid_trigger_actuation_menu(enum Menu state) {
  char *actuation_prefix = " a = ";
  char *press_prefix = " e = ";
  char *release_prefix = " l = ";
  char *actuation_suffix = EMPTY_SUFFIX;
  char *press_suffix = EMPTY_SUFFIX;
  char *release_suffix = EMPTY_SUFFIX;
  char *changes = "+";

  if (kb_config.global_actuation_settings.actuation_point_dmm ==
      stored_kb_config.global_actuation_settings.actuation_point_dmm) {
    changes = " ";
  }

  if (state == SET_ACTUATION) {
    actuation_prefix = SELECT_PREFIX;
    actuation_suffix = SELECT_SUFFIX;
  } else if (state == SET_PRESS_SENSITIVITY) {
    press_prefix = SELECT_PREFIX;
    press_suffix = SELECT_SUFFIX;
  } else if (state == SET_RELEASE_SENSITIVITY) {
    release_prefix = SELECT_PREFIX;
    release_suffix = SELECT_SUFFIX;
  }

  char *menu_strings[] = {NL,
                          " MAIN MENU -> ACTUATION SETTINGS",
                          NL,
                          NL,
                          PER_KEY_SETTINGS_OFF_STR,
                          NL,
                          NL,
                          RAPID_TRIGGER_ON_STR,
                          NL,
                          NL,
                          NL,
                          actuation_prefix,
                          "Actuation Distance   ",
                          actuation_setting_bar,
                          actuation_suffix,
                          NL,
                          NL,
                          press_prefix,
                          "Press Sensitivity    ",
                          press_setting_bar,
                          press_suffix,
                          NL,
                          NL,
                          release_prefix,
                          "Release Sensitivity  ",
                          release_setting_bar,
                          release_suffix,
                          NL,
                          NL,
                          NL,
                          " i = +1",
                          NL,
                          NL,
                          " d = -1",
                          NL,
                          NL,
                          " I = +5",
                          NL,
                          NL,
                          " D = -5",
                          NL,
                          NL,
                          " c = Clear Changes",
                          NL,
                          NL,
                          " s = Save Settings",
                          changes,
                          NL,
                          NL,
                          " b = Back <-",
                          NL,
                          NL,
                          NULL};
  print_strings_serial(menu_strings);
  cursor_right();
}

void print_lighting_menu(void) {
  char *effect_string = lighting_mode_string();
  char *menu_strings[] = {NL,
                          " MAIN MENU -> LIGHTING SETTINGS",
                          NL,
                          NL,
                          " e = Effect",
                          "   < ",
                          effect_string,
                          " > ",
                          NL,
                          NL,
                          " h = Hue         ",
                          hue_setting_bar,
                          NL,
                          NL,
                          " a = Saturation  ",
                          saturation_setting_bar,
                          NL,
                          NL,
                          " r = Brightness  ",
                          brightness_setting_bar,
                          NL,
                          NL,
                          " p = Speed       ",
                          speed_setting_bar,
                          NL,
                          NL,
                          " c = Clear Changes",
                          NL,
                          NL,
                          " s = Save Settings",
                          NL,
                          NL,
                          " b = Back <-",
                          NL,
                          NL,
                          NULL};
  print_strings_serial(menu_strings);
  cursor_right();
}

void display_menu(enum Menu state) {
  switch (state) {
  case MAIN:
    print_main_menu();
    return;
  case SET_ACTUATION:
  case SET_PRESS_SENSITIVITY:
  case SET_RELEASE_SENSITIVITY:
    if (kb_config.use_per_key_settings) {
      print_per_key_actuation_menu();
      return;
    }
    if (kb_config.global_actuation_settings.rapid_trigger) {
      print_rapid_trigger_actuation_menu(state);
      return;
    }
    print_standard_actuation_menu();
    return;
  case LIGHTING:
  case SET_EFFECT:
  case SET_SPEED:
  case SET_BRIGHTNESS:
  case SET_COLOR:
    print_lighting_menu();
    break;
  case KEYMAP:
    break;
  case RESTORE_DEFAULT:
    break;
  }
}

void handle_menu(const uint8_t ch) {
  static enum Menu state = MAIN;

  switch (state) {
  case MAIN:
    if (ch == 'a' || ch == 'A') {
      state = SET_ACTUATION;
    } else if (ch == 'l' || ch == 'L') {
      state = LIGHTING;
    }
    break;
  case SET_ACTUATION:
    if (ch == 'b' || ch == 'B') {
      state = MAIN;
    } else if (ch == 'e' || ch == 'E') {
      // can only change press and release sensitivity when rapid trigger is
      // on
      if (kb_config.global_actuation_settings.rapid_trigger) {
        state = SET_PRESS_SENSITIVITY;
      }
    } else if (ch == 'l' || ch == 'L') {
      // can only change press and release sensitivity when rapid trigger is
      // on
      if (kb_config.global_actuation_settings.rapid_trigger) {
        state = SET_RELEASE_SENSITIVITY;
      }
    } else if (ch == 'p' || ch == 'P') {
      kb_config.use_per_key_settings = !kb_config.use_per_key_settings;
      state = SET_ACTUATION;
    } else if (ch == 'r' || ch == 'R') {
      kb_config.global_actuation_settings.rapid_trigger =
          !kb_config.global_actuation_settings.rapid_trigger;
    } else if (ch == 'i') {
      kb_config.global_actuation_settings.actuation_point_dmm =
          clamp_setpoint_dmm(
              kb_config.global_actuation_settings.actuation_point_dmm + 1);
      create_2_digit_setting_bar(
          actuation_setting_bar,
          kb_config.global_actuation_settings.actuation_point_dmm);
    } else if (ch == 'd') {
      kb_config.global_actuation_settings.actuation_point_dmm =
          clamp_setpoint_dmm(
              kb_config.global_actuation_settings.actuation_point_dmm - 1);
      create_2_digit_setting_bar(
          actuation_setting_bar,
          kb_config.global_actuation_settings.actuation_point_dmm);
    } else if (ch == 'I') {
      kb_config.global_actuation_settings.actuation_point_dmm =
          clamp_setpoint_dmm(
              kb_config.global_actuation_settings.actuation_point_dmm + 5);
      create_2_digit_setting_bar(
          actuation_setting_bar,
          kb_config.global_actuation_settings.actuation_point_dmm);
    } else if (ch == 'D') {
      kb_config.global_actuation_settings.actuation_point_dmm =
          clamp_setpoint_dmm(
              kb_config.global_actuation_settings.actuation_point_dmm - 5);
      create_2_digit_setting_bar(
          actuation_setting_bar,
          kb_config.global_actuation_settings.actuation_point_dmm);
    }
    break;
  case SET_PRESS_SENSITIVITY:
    if (ch == 'b' || ch == 'B') {
      state = MAIN;
    } else if (ch == 'a' || ch == 'A') {
      state = SET_ACTUATION;
    } else if (ch == 'l' || ch == 'L') {
      state = SET_RELEASE_SENSITIVITY;
    } else if (ch == 'p' || ch == 'P') {
      kb_config.use_per_key_settings = !kb_config.use_per_key_settings;
      state = SET_ACTUATION;
    } else if (ch == 'r' || ch == 'R') {
      kb_config.global_actuation_settings.rapid_trigger =
          !kb_config.global_actuation_settings.rapid_trigger;
      state = SET_ACTUATION;
    } else if (ch == 'i') {
      kb_config.global_actuation_settings.rapid_trigger_press_sensitivity_dmm =
          clamp_setpoint_dmm(kb_config.global_actuation_settings
                                 .rapid_trigger_press_sensitivity_dmm +
                             1);
      create_2_digit_setting_bar(press_setting_bar,
                                 kb_config.global_actuation_settings
                                     .rapid_trigger_press_sensitivity_dmm);
    } else if (ch == 'd') {
      kb_config.global_actuation_settings.rapid_trigger_press_sensitivity_dmm =
          clamp_setpoint_dmm(kb_config.global_actuation_settings
                                 .rapid_trigger_press_sensitivity_dmm -
                             1);
      create_2_digit_setting_bar(press_setting_bar,
                                 kb_config.global_actuation_settings
                                     .rapid_trigger_press_sensitivity_dmm);
    } else if (ch == 'I') {
      kb_config.global_actuation_settings.rapid_trigger_press_sensitivity_dmm =
          clamp_setpoint_dmm(kb_config.global_actuation_settings
                                 .rapid_trigger_press_sensitivity_dmm +
                             5);
      create_2_digit_setting_bar(press_setting_bar,
                                 kb_config.global_actuation_settings
                                     .rapid_trigger_press_sensitivity_dmm);
    } else if (ch == 'D') {
      kb_config.global_actuation_settings.rapid_trigger_press_sensitivity_dmm =
          clamp_setpoint_dmm(kb_config.global_actuation_settings
                                 .rapid_trigger_press_sensitivity_dmm -
                             5);
      create_2_digit_setting_bar(press_setting_bar,
                                 kb_config.global_actuation_settings
                                     .rapid_trigger_press_sensitivity_dmm);
    }
    break;
  case SET_RELEASE_SENSITIVITY:
    if (ch == 'b' || ch == 'B') {
      state = MAIN;
    } else if (ch == 'a' || ch == 'A') {
      state = SET_ACTUATION;
    } else if (ch == 'e' || ch == 'E') {
      state = SET_PRESS_SENSITIVITY;
    } else if (ch == 'p' || ch == 'P') {
      kb_config.use_per_key_settings = !kb_config.use_per_key_settings;
      state = SET_ACTUATION;
    } else if (ch == 'r' || ch == 'R') {
      kb_config.global_actuation_settings.rapid_trigger =
          !kb_config.global_actuation_settings.rapid_trigger;
      state = SET_ACTUATION;
    } else if (ch == 'i') {
      kb_config.global_actuation_settings
          .rapid_trigger_release_sensitivity_dmm =
          clamp_setpoint_dmm(kb_config.global_actuation_settings
                                 .rapid_trigger_release_sensitivity_dmm +
                             1);
      create_2_digit_setting_bar(release_setting_bar,
                                 kb_config.global_actuation_settings
                                     .rapid_trigger_release_sensitivity_dmm);
    } else if (ch == 'd') {
      kb_config.global_actuation_settings
          .rapid_trigger_release_sensitivity_dmm =
          clamp_setpoint_dmm(kb_config.global_actuation_settings
                                 .rapid_trigger_release_sensitivity_dmm -
                             1);
      create_2_digit_setting_bar(release_setting_bar,
                                 kb_config.global_actuation_settings
                                     .rapid_trigger_release_sensitivity_dmm);
    } else if (ch == 'I') {
      kb_config.global_actuation_settings
          .rapid_trigger_release_sensitivity_dmm =
          clamp_setpoint_dmm(kb_config.global_actuation_settings
                                 .rapid_trigger_release_sensitivity_dmm +
                             5);
      create_2_digit_setting_bar(release_setting_bar,
                                 kb_config.global_actuation_settings
                                     .rapid_trigger_release_sensitivity_dmm);
    } else if (ch == 'D') {
      kb_config.global_actuation_settings
          .rapid_trigger_release_sensitivity_dmm =
          clamp_setpoint_dmm(kb_config.global_actuation_settings
                                 .rapid_trigger_release_sensitivity_dmm -
                             5);
      create_2_digit_setting_bar(release_setting_bar,
                                 kb_config.global_actuation_settings
                                     .rapid_trigger_release_sensitivity_dmm);
    }
    break;
  case LIGHTING:
    if (ch == 'e' || ch == 'E') {
      state = SET_EFFECT;
    }
    if (ch == 'p' || ch == 'P') {
      state = SET_SPEED;
    }
    if (ch == 'r' || ch == 'R') {
      state = SET_BRIGHTNESS;
    }
    if (ch == 'c' || ch == 'C') {
      state = SET_COLOR;
    }
    if (ch == 'b' || ch == 'B') {
      state = MAIN;
    }
    break;
  case SET_EFFECT:
    if (ch == 'b' || ch == 'B') {
      state = MAIN;
    }
    break;
  case SET_SPEED:
    if (ch == 'b' || ch == 'B') {
      state = MAIN;
    }
    break;
  case SET_BRIGHTNESS:
    if (ch == 'b' || ch == 'B') {
      state = MAIN;
    }
    break;
  case SET_COLOR:
    if (ch == 'b' || ch == 'B') {
      state = MAIN;
    }
    break;
  case KEYMAP:
    if (ch == 'b' || ch == 'B') {
      state = MAIN;
    }
    break;
  case RESTORE_DEFAULT:

    break;
  }
  if (ch == 's' || ch == 'S') {
    kb_config_save_to_eeprom();
  } else if (ch == 'c' || ch == 'C') {
    kb_config_reload_from_eeprom();
    // TODO: only init the setting bars that have changed
    serial_configurator_init_setting_bars();
  }

  // TODO: only reset_terminal when needed
  reset_terminal();

  display_menu(state);
}

void virtser_recv(const uint8_t ch) {
  if (ch == '-') {
    bootloader_jump();
  }
  handle_menu(ch);
  dprintf("virtser_recv: ch: %3u \n", ch);
}

void serial_configurator_init_setting_bars(void) {
  create_2_digit_setting_bar(
      actuation_setting_bar,
      kb_config.global_actuation_settings.actuation_point_dmm);

  create_2_digit_setting_bar(
      press_setting_bar,
      kb_config.global_actuation_settings.rapid_trigger_press_sensitivity_dmm);

  create_2_digit_setting_bar(release_setting_bar,
                             kb_config.global_actuation_settings
                                 .rapid_trigger_release_sensitivity_dmm);

  create_3_digit_setting_bar(speed_setting_bar, rgb_matrix_get_speed());
  create_3_digit_setting_bar(hue_setting_bar, rgb_matrix_get_hue());
  create_3_digit_setting_bar(saturation_setting_bar, rgb_matrix_get_sat());
  create_3_digit_setting_bar(brightness_setting_bar, rgb_matrix_get_val());
}
