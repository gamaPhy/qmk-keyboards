#include "quantum.h"
#include "virtser.h"

#include "ascii_art.h"
#include "serial_configurator.h"
#include <stdio.h>

kb_config_t kb_config;

enum Menu {
  MAIN,
  INPUT_ACTUATION,
  INPUT_PRESS_SENSITIVITY,
  INPUT_RELEASE_SENSITIVITY,
  ACTUATION_PER_KEY,
  KEYMAP,
  LIGHTING,
  INPUT_EFFECT,
  INPUT_SPEED,
  INPUT_BRIGHTNESS,
  INPUT_COLOR,
  RESTORE_DEFAULT,
};

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
  cursor_right();
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
}

int clamp_setpoint_dmm(int setpoint_dmm) {
  if (setpoint_dmm < 1) {
    return 1;
  } else if (setpoint_dmm > KEY_MAX_dmm) {
    return KEY_MAX_dmm;
  } else {
    return setpoint_dmm;
  }
}

// Sets the contents of `setting_bar` to a bar that fills with values 1 - 40
// Ex:
// 20 <@@@@@@@@@__________>
// 40 <@@@@@@@@@@@@@@@@@@@>
//  1 <@__________________>
//
// `setting_bar` must have a size of SETTING_BAR_SIZE + 2 to fit the potentially
// 2 digit number
void create_2_digit_setting_bar(char *setting_bar, int setpoint) {
  // Maximum actuation distance is a 2 digit integer pad with leading spaces if
  // there is a 1 digit integer to align setting bar
  char setting_num_str[2];
  sprintf(setting_num_str, "%2d", setpoint);

  char setting_fill[SETTING_BAR_SIZE];
  setting_fill[0] = ' ';
  setting_fill[1] = '<';
  int i;
  int fill_to = 0;
  int scaled_setpoint = setpoint / 2;
  for (i = 2; i <= (KEY_MAX_dmm / 2) + 1; i++) {
    fill_to++;
    if (fill_to <= scaled_setpoint) {
      setting_fill[i] = '@';
    } else
      setting_fill[i] = '_';
  }
  setting_fill[i] = '>';
  setting_fill[i + 1] = '\0';
  strcpy(setting_bar, setting_num_str);
  strcat(setting_bar, setting_fill);
}

// Sets the contents of `setting_bar` to a bar that fills with values 1 - 255
// Ex:
// 127 <@@@@@@@@@__________>
// 255 <@@@@@@@@@@@@@@@@@@@>
//   1 <@__________________>
//
// `setting_bar` must have a size of SETTING_BAR_SIZE + 3 to fit the potentially
// 3 digit number
void create_3_digit_setting_bar(char *setting_bar, int setpoint) {
  // Maximum value for some settings is a 3 digit number pad with leading spaces
  // if there is a 1 or 2 digit integer to align setting bar
  char setting_num_str[3];
  sprintf(setting_num_str, "%3d", setpoint);

  char setting_fill[SETTING_BAR_SIZE];
  setting_fill[0] = ' ';
  setting_fill[1] = '<';
  int i;
  int fill_to = 0;
  int scaled_setpoint = (setpoint * 20) / 255;
  for (i = 2; i <= (KEY_MAX_dmm / 2) + 1; i++) {
    fill_to++;
    if (fill_to <= scaled_setpoint) {
      setting_fill[i] = '@';
    } else
      setting_fill[i] = '_';
  }
  setting_fill[i] = '>';
  setting_fill[i + 1] = '\0';
  strcpy(setting_bar, setting_num_str);
  strcat(setting_bar, setting_fill);
}

void print_actuation_menu(enum Menu state, char *actuation_setting_bar,
                          char *press_setting_bar, char *release_setting_bar) {
  char *per_key_settings;
  char *rapid_trigger_setting;
  char *actuation_prefix = " a = ";
  char *press_prefix = " e = ";
  char *release_prefix = " l = ";

  if (state == INPUT_ACTUATION) {
    actuation_prefix = " --> ";
  } else if (state == INPUT_PRESS_SENSITIVITY) {
    press_prefix = " --> ";
  } else if (state == INPUT_RELEASE_SENSITIVITY) {
    release_prefix = " --> ";
  }

  if (kb_config.use_per_key_settings) {
    per_key_settings = " <ON>";
    char *menu_strings[] = {NL,
                            " MAIN MENU -> ACTUATION SETTINGS",
                            NL,
                            NL,
                            " p = Per-Key Settings ",
                            per_key_settings,
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
                            NULL};
    print_strings_serial(menu_strings);
  } else {
    per_key_settings = " <OFF>";
    // Whitespace offset to align with per-key settings
    if (kb_config.global_actuation_settings.rapid_trigger) {
      rapid_trigger_setting = "    <ON>";
      char *menu_strings[] = {NL,
                              " MAIN MENU -> ACTUATION SETTINGS",
                              NL,
                              NL,
                              " p = Per-Key Settings ",
                              per_key_settings,
                              NL,
                              NL,
                              " r = Rapid Trigger ",
                              rapid_trigger_setting,
                              NL,
                              NL,
                              NL,
                              actuation_prefix,
                              "Actuation Distance   ",
                              actuation_setting_bar,
                              NL,
                              NL,
                              press_prefix,
                              "Press Sensitivity    ",
                              press_setting_bar,
                              NL,
                              NL,
                              release_prefix,
                              "Release Sensitivity  ",
                              release_setting_bar,
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
                              " s = Save Settings",
                              NL,
                              NL,
                              " b = Back <-",
                              NL,
                              NULL};
      print_strings_serial(menu_strings);
    } else {
      rapid_trigger_setting = "    <OFF>";
      char *menu_strings[] = {NL,
                              " MAIN MENU -> ACTUATION SETTINGS",
                              NL,
                              NL,
                              " p = Per-Key Settings ",
                              per_key_settings,
                              NL,
                              NL,
                              " r = Rapid Trigger ",
                              rapid_trigger_setting,
                              NL,
                              NL,
                              NL,
                              " a = Actuation Distance   ",
                              actuation_setting_bar,
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
                              " s = Save Settings",
                              NL,
                              NL,
                              " b = Back <-",
                              NL,
                              NULL};
      print_strings_serial(menu_strings);
    }
  }

  cursor_down();
}

void print_lighting_menu(char *brightness_setting_bar,
                         char *speed_setting_bar) {
  char *menu_strings[] = {NL,
                          " MAIN MENU -> LIGHTING SETTINGS",
                          NL,
                          NL,
                          " e = Effect",
                          NL,
                          NL,
                          " c = Color",
                          NL,
                          NL,
                          " r = Brightness  ",
                          brightness_setting_bar,
                          NL,
                          NL,
                          " s = Speed       ",
                          speed_setting_bar,
                          NL,
                          NL,
                          " b = Back <-",
                          NL,
                          NULL};
  print_strings_serial(menu_strings);
  cursor_down();
}

void display_menu(enum Menu state, int new_setpoint_dmm) {
  reset_terminal();

  if (state == MAIN) {
    print_main_menu();
  } else if (state == ACTUATION_PER_KEY || state == INPUT_ACTUATION ||
             state == INPUT_PRESS_SENSITIVITY ||
             state == INPUT_RELEASE_SENSITIVITY) {
    char actuation_setting_bar[SETTING_BAR_SIZE + 2];
    create_2_digit_setting_bar(
        actuation_setting_bar,
        kb_config.global_actuation_settings.actuation_point_dmm);

    char press_setting_bar[SETTING_BAR_SIZE + 2];
    create_2_digit_setting_bar(press_setting_bar,
                               kb_config.global_actuation_settings
                                   .rapid_trigger_press_sensitivity_dmm);

    char release_setting_bar[SETTING_BAR_SIZE + 2];
    create_2_digit_setting_bar(release_setting_bar,
                               kb_config.global_actuation_settings
                                   .rapid_trigger_release_sensitivity_dmm);

    print_actuation_menu(state, actuation_setting_bar, press_setting_bar,
                         release_setting_bar);

    if (state == INPUT_ACTUATION) {
    } else if (state == INPUT_PRESS_SENSITIVITY) {
    } else if (state == INPUT_RELEASE_SENSITIVITY) {
    }

  } else if (state == LIGHTING || state == INPUT_BRIGHTNESS ||
             state == INPUT_EFFECT || state == INPUT_SPEED ||
             state == INPUT_COLOR) {
    char speed_setting_bar[SETTING_BAR_SIZE + 3];
    create_3_digit_setting_bar(speed_setting_bar, rgb_matrix_get_speed());

    char brightness_setting_bar[SETTING_BAR_SIZE + 3];
    create_3_digit_setting_bar(brightness_setting_bar, rgb_matrix_get_val());

    print_lighting_menu(brightness_setting_bar, speed_setting_bar);
  } else if (state == KEYMAP) {
  } else if (state == RESTORE_DEFAULT) {
    // Do nothing (no operation for RESTORE_DEFAULT)
  } else {
    // Do nothing
  }
}

void handle_menu(const uint16_t ch) {
  static enum Menu state = MAIN;
  static int new_setpoint_dmm = 0;

  switch (state) {
  case MAIN:
    if (ch == 'a' || ch == 'A') {
      if (kb_config.use_per_key_settings) {
        state = ACTUATION_PER_KEY;
      } else {
        state = INPUT_ACTUATION;
        new_setpoint_dmm =
            kb_config.global_actuation_settings.actuation_point_dmm;
      }
    } else if (ch == 'l' || ch == 'L') {
      state = LIGHTING;
    }
    break;
  case ACTUATION_PER_KEY:
    if (ch == 'b' || ch == 'B') {
      state = MAIN;
    } else if (ch == 'p' || ch == 'P') {
      kb_config.use_per_key_settings = !kb_config.use_per_key_settings;
      state = INPUT_ACTUATION;
      new_setpoint_dmm =
          kb_config.global_actuation_settings.actuation_point_dmm;
    } else if (ch == 's' || ch == 'S') {
      eeconfig_update_kb_datablock(&kb_config);
    }
    break;
  // 'OR' logic equivalent for switch statements
  case INPUT_ACTUATION:
  case INPUT_PRESS_SENSITIVITY:
  case INPUT_RELEASE_SENSITIVITY:
    // can only change press and release sensitivity when rapid trigger is on
    if (kb_config.global_actuation_settings.rapid_trigger) {
      if (ch == 'e' || ch == 'E') {
        state = INPUT_PRESS_SENSITIVITY;
        new_setpoint_dmm = kb_config.global_actuation_settings
                               .rapid_trigger_press_sensitivity_dmm;
      } else if (ch == 'l' || ch == 'L') {
        state = INPUT_RELEASE_SENSITIVITY;
        new_setpoint_dmm = kb_config.global_actuation_settings
                               .rapid_trigger_release_sensitivity_dmm;
      }
    }
    if (ch == 'b' || ch == 'B') {
      state = MAIN;
    } else if (ch == 'p' || ch == 'P') {
      kb_config.use_per_key_settings = !kb_config.use_per_key_settings;
      state = ACTUATION_PER_KEY;
    } else if (ch == 'r' || ch == 'R') {
      kb_config.global_actuation_settings.rapid_trigger =
          !kb_config.global_actuation_settings.rapid_trigger;
    } else if (ch == 'a' || ch == 'A') {
      state = INPUT_ACTUATION;
      new_setpoint_dmm =
          kb_config.global_actuation_settings.actuation_point_dmm;
    } else if (ch == 'i') {
      new_setpoint_dmm = clamp_setpoint_dmm(new_setpoint_dmm + 1);
    } else if (ch == 'd') {
      new_setpoint_dmm = clamp_setpoint_dmm(new_setpoint_dmm - 1);
    } else if (ch == 'I') {
      new_setpoint_dmm = clamp_setpoint_dmm(new_setpoint_dmm + 5);
    } else if (ch == 'D') {
      new_setpoint_dmm = clamp_setpoint_dmm(new_setpoint_dmm - 5);
    }
    if (state == INPUT_ACTUATION) {
      kb_config.global_actuation_settings.actuation_point_dmm =
          new_setpoint_dmm;
    } else if (state == INPUT_RELEASE_SENSITIVITY) {
      kb_config.global_actuation_settings
          .rapid_trigger_release_sensitivity_dmm = new_setpoint_dmm;
    } else if (state == INPUT_PRESS_SENSITIVITY) {
      kb_config.global_actuation_settings.rapid_trigger_press_sensitivity_dmm =
          new_setpoint_dmm;
    }
    if (ch == 's' || ch == 'S') {
      eeconfig_update_kb_datablock(&kb_config);
    }
    break;
  case LIGHTING:
    if (ch == 'e' || ch == 'E') {
      state = INPUT_EFFECT;
    }
    if (ch == 's' || ch == 'S') {
      state = INPUT_SPEED;
    }
    if (ch == 'r' || ch == 'R') {
      state = INPUT_BRIGHTNESS;
    }
    if (ch == 'c' || ch == 'C') {
      state = INPUT_COLOR;
    }
    if (ch == 'b' || ch == 'B') {
      state = MAIN;
    }
    break;
  case INPUT_EFFECT:
    if (ch == 'b' || ch == 'B') {
      state = MAIN;
    }
    break;
  case INPUT_SPEED:
    if (ch == 'b' || ch == 'B') {
      state = MAIN;
    }
    break;
  case INPUT_BRIGHTNESS:
    if (ch == 'b' || ch == 'B') {
      state = MAIN;
    }
    break;
  case INPUT_COLOR:
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
  display_menu(state, new_setpoint_dmm);
}

void virtser_recv(const uint8_t ch) {
  static bool escaping = false;
  dprintf("virtser_recv: ch: %3u \n", ch);
  // Handling of arrow characters
  if (ch == ESC) {
    escaping = true;
  } else if (escaping == true && ch == '[') {
    // consume intermediate '[' character received for arrow commands
  } else if (escaping) {
    switch (ch) {
    case 'A':
      handle_menu(ARROW_UP);
      break;
    case 'B':
      handle_menu(ARROW_DOWN);
      break;
    case 'C':
      handle_menu(ARROW_RIGHT);
      break;
    case 'D':
      handle_menu(ARROW_LEFT);
      break;
    }
    escaping = false;
  } else {
    handle_menu(ch);
    escaping = false;
  }
}
