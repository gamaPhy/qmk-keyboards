#include "quantum.h"
#include "virtser.h"

#include "ascii_art.h"
#include "serial_configurator.h"
#include <stdio.h>

kb_config_t kb_config;

enum Menu {
  MAIN,
  ACTUATION,
  INPUT_ACTUATION,
  INPUT_PRESS_SENSITIVITY,
  INPUT_RELEASE_SENSITIVITY,
  KEYMAP,
  LIGHTING,
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
                          " [A] Actuation",
                          NL,
                          NL,
                          " [L] Lighting",
                          NL,
                          NL,
                          " [K] Keymap",
                          NL,
                          NL,
                          NULL};

  print_strings_serial(menu_strings);
}

void create_setting_bar(char *setting_bar, int setpoint) {
  char setting_num_str[2];
  sprintf(setting_num_str, "%2d", setpoint);

  // setting bar size less the two digit number in front
  char setting_fill[SETTING_BAR_SIZE - 2];
  setting_fill[0] = ' ';
  setting_fill[1] = '<';
  int i;
  int fill_to = 0;
  for (i = 2; i <= KEY_MAX_dmm + 1; i++) {
    fill_to++;
    if (fill_to <= setpoint) {
      setting_fill[i] = '@';
    } else
      setting_fill[i] = '_';
  }
  setting_fill[i] = '>';
  setting_fill[i + 1] = '\0';
  strcpy(setting_bar, setting_num_str);
  strcat(setting_bar, setting_fill);
}

void print_actuation_menu(char *actuation_setting_bar, char *press_setting_bar,
                          char *release_setting_bar) {
  char *per_key_settings;
  char *rapid_trigger_setting;

  if (kb_config.use_per_key_settings) {
    per_key_settings = " <ON>";
    char *menu_strings[] = {NL,
                            " MAIN MENU -> ACTUATION SETTINGS",
                            NL,
                            NL,
                            " [P] Per-Key Settings ",
                            per_key_settings,
                            NL,
                            NL,
                            " [B] Back <-",
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
                              " [P] Per-Key Settings ",
                              per_key_settings,
                              NL,
                              NL,
                              " [R] Rapid Trigger ",
                              rapid_trigger_setting,
                              NL,
                              NL,
                              " [A] Actuation Distance  ",
                              actuation_setting_bar,
                              NL,
                              NL,
                              " [E] Press Sensitivity   ",
                              press_setting_bar,
                              NL,
                              NL,
                              " [L] Release Sensitivity ",
                              release_setting_bar,
                              NL,
                              NL,
                              " [B] Back <-",
                              NL,
                              NULL};
      print_strings_serial(menu_strings);
    } else {
      rapid_trigger_setting = "    <OFF>";
      char *menu_strings[] = {NL,
                              " MAIN MENU -> ACTUATION SETTINGS",
                              NL,
                              NL,
                              " [P] Per-Key Settings ",
                              per_key_settings,
                              NL,
                              NL,
                              " [R] Rapid Trigger ",
                              rapid_trigger_setting,
                              NL,
                              NL,
                              " [A] Actuation Distance  ",
                              actuation_setting_bar,
                              NL,
                              NL,
                              " [B] Back <-",
                              NL,
                              NULL};
      print_strings_serial(menu_strings);
    }
  }

  cursor_down();
}

void print_set_new_setpoint(char *setting_name_uppercase, char *setting_name,
                            char *setting_bar, char *prompt,
                            int new_setpoint_dmm) {
  char nice[69] = {'\0'};
  if (new_setpoint_dmm == 69) {
    strcpy(nice, "              Nice.");
  }
  char new_setpoint_str[2] = {'\0'};
  if (new_setpoint_dmm != 0) {
    sprintf(new_setpoint_str, "%d", new_setpoint_dmm);
  }

  char new_setting_bar[SETTING_BAR_SIZE];
  create_setting_bar(new_setting_bar, new_setpoint_dmm);

  char *menu_strings[] = {NL,
                          " ---------------------------------------------------"
                          "-------------------------",
                          NL,
                          NL,
                          setting_name_uppercase,
                          NL,
                          NL,
                          " [I] Increase",
                          NL,
                          NL,
                          " [D] Decrease",
                          NL,
                          NL,
                          " [S] Save New Setting",
                          NL,
                          NL,
                          " [X] Close",
                          nice,
                          NL,
                          NL,
                          NL,
                          setting_name,
                          NL,
                          NL,
                          " Current ",
                          setting_bar,
                          NL,
                          NL,
                          "     New ",
                          new_setting_bar,
                          NL,
                          NL,
                          NL,
                          prompt,
                          new_setpoint_str,
                          NULL};
  print_strings_serial(menu_strings);
  cursor_left();
}

void print_lighting_menu(void) {
  char *menu_strings[] = {NL,
                          " MAIN MENU -> LIGHTING SETTINGS",
                          NL,
                          NL,
                          " [E] Effect",
                          NL,
                          NL,
                          " [S] Speed",
                          NL,
                          NL,
                          " [R] Brightness",
                          NL,
                          NL,
                          " [C] Color",
                          NL,
                          NL,
                          " [B] Back <-",
                          NL,
                          NULL};
  print_strings_serial(menu_strings);
  cursor_down();
}

void display_menu(enum Menu state, int new_setpoint_dmm) {
  reset_terminal();

  if (state == MAIN) {
    print_main_menu();
  } else if (state == ACTUATION || state == INPUT_ACTUATION ||
             state == INPUT_PRESS_SENSITIVITY ||
             state == INPUT_RELEASE_SENSITIVITY) {
    // Maximum actuation distance is a 2 digit integer
    // pad with leading spaces if there is a 1 digit integer to align setting
    // bar
    char actuation_setting_bar[SETTING_BAR_SIZE];
    create_setting_bar(actuation_setting_bar,
                       kb_config.global_actuation_settings.actuation_point_dmm);

    char press_setting_bar[SETTING_BAR_SIZE];
    create_setting_bar(press_setting_bar,
                       kb_config.global_actuation_settings
                           .rapid_trigger_press_sensitivity_dmm);

    char release_setting_bar[SETTING_BAR_SIZE];
    create_setting_bar(release_setting_bar,
                       kb_config.global_actuation_settings
                           .rapid_trigger_release_sensitivity_dmm);

    print_actuation_menu(actuation_setting_bar, press_setting_bar,
                         release_setting_bar);

    if (state == INPUT_ACTUATION) {
      print_set_new_setpoint(" ACTUATION DISTANCE SETTING",
                             " Actuation Distance", actuation_setting_bar,
                             " Input (1-40): ", new_setpoint_dmm);
    } else if (state == INPUT_PRESS_SENSITIVITY) {
      print_set_new_setpoint(" PRESS SENSITIVITY SETTING", " Press Sensitivity",
                             press_setting_bar,
                             " Input (1-40): ", new_setpoint_dmm);
    } else if (state == INPUT_RELEASE_SENSITIVITY) {
      print_set_new_setpoint(" RELEASE SENSITIVITY SETTING",
                             " Release Sensitivity", release_setting_bar,
                             " Input (1-40): ", new_setpoint_dmm);
    }
  } else if (state == LIGHTING) {
    print_lighting_menu();
  } else if (state == KEYMAP) {
  } else if (state == RESTORE_DEFAULT) {
    // Do nothing (no operation for RESTORE_DEFAULT)
  } else {
    // Handle any other cases if necessary
  }
}

bool setpoint_valid(int setpoint_dmm) {
  if (1 <= setpoint_dmm && setpoint_dmm <= KEY_MAX_dmm) {
    return true;
  }
  return false;
}

void handle_menu(const uint16_t ch) {
  static enum Menu state = MAIN;
  static int new_setpoint_dmm = 0;

  switch (state) {
  case MAIN:
    if (ch == 'a' || ch == 'A') {
      state = ACTUATION;
    } else if (ch == 'l' || ch == 'L') {
      state = LIGHTING;
    }
    break;
  case ACTUATION:
    if (ch == 'b' || ch == 'B') {
      state = MAIN;
    } else if (ch == 'p' || ch == 'P') {
      kb_config.use_per_key_settings = !kb_config.use_per_key_settings;
      eeconfig_update_kb_datablock(&kb_config);
    }
    if (!kb_config.use_per_key_settings) {
      // only allow changing global settings when per-key settings are off
      if (ch == 'r' || ch == 'R') {
        kb_config.global_actuation_settings.rapid_trigger =
            !kb_config.global_actuation_settings.rapid_trigger;
        eeconfig_update_kb_datablock(&kb_config);
      } else if (ch == 'a' || ch == 'A') {
        state = INPUT_ACTUATION;
        new_setpoint_dmm =
            kb_config.global_actuation_settings.actuation_point_dmm;
      }

      // can only change press and release sensitivity when rapid trigger is
      // on
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
    }
    break;
  // 'OR' logic equivalent for switch statements
  case INPUT_ACTUATION:
  case INPUT_PRESS_SENSITIVITY:
  case INPUT_RELEASE_SENSITIVITY:
    if (ch == 'b' || ch == 'B') {
      state = MAIN;
    } else if (ch == 'x' || ch == 'X') {
      // cancelling setting the setpoint
      state = ACTUATION;
      new_setpoint_dmm = 0;
    } else if (ch == 'p' || ch == 'P') {
      kb_config.use_per_key_settings = !kb_config.use_per_key_settings;
      eeconfig_update_kb_datablock(&kb_config);
      state = ACTUATION;
    } else if (ch == 'r' || ch == 'R') {
      kb_config.global_actuation_settings.rapid_trigger =
          !kb_config.global_actuation_settings.rapid_trigger;
      eeconfig_update_kb_datablock(&kb_config);
      state = ACTUATION;
    } else if (ch == 'a' || ch == 'A') {
      state = INPUT_ACTUATION;
      new_setpoint_dmm =
          kb_config.global_actuation_settings.actuation_point_dmm;
    } else if (ch == 'e' || ch == 'E') {
      state = INPUT_PRESS_SENSITIVITY;
      new_setpoint_dmm = kb_config.global_actuation_settings
                             .rapid_trigger_press_sensitivity_dmm;
    } else if (ch == 'l' || ch == 'L') {
      state = INPUT_RELEASE_SENSITIVITY;
      new_setpoint_dmm = kb_config.global_actuation_settings
                             .rapid_trigger_release_sensitivity_dmm;
    } else if (ch == 'i' || ch == 'I') {
      if (setpoint_valid(new_setpoint_dmm + 1)) {
        new_setpoint_dmm++;
      }
    } else if (ch == 'd' || ch == 'D') {
      if (setpoint_valid(new_setpoint_dmm - 1)) {
        new_setpoint_dmm--;
      }
    } else if ('0' <= ch && ch <= '9') {
      // reset if input would exceed double digit number
      if (new_setpoint_dmm * 10 + ch - '0' > 99) {
        new_setpoint_dmm = 0;
      }
      if (new_setpoint_dmm == 0) {
        if ('1' <= ch && ch <= '9') {
          new_setpoint_dmm = ch - '0';
        }
      } else {
        // entering second digit
        new_setpoint_dmm = new_setpoint_dmm * 10 + ch - '0';
      }
    } else if (ch == BS) {
      if (new_setpoint_dmm >= 10) {
        new_setpoint_dmm = new_setpoint_dmm / 10;
      } else {
        new_setpoint_dmm = 0;
      }
    } else if (ch == DEL) {
      new_setpoint_dmm = 0;
    } else if (ch == 's' || ch == 'S') {
      if (setpoint_valid(new_setpoint_dmm)) {
        if (state == INPUT_ACTUATION) {
          kb_config.global_actuation_settings.actuation_point_dmm =
              new_setpoint_dmm;
        } else if (state == INPUT_RELEASE_SENSITIVITY) {
          kb_config.global_actuation_settings
              .rapid_trigger_release_sensitivity_dmm = new_setpoint_dmm;
        } else if (state == INPUT_PRESS_SENSITIVITY) {
          kb_config.global_actuation_settings
              .rapid_trigger_press_sensitivity_dmm = new_setpoint_dmm;
        }
        eeconfig_update_kb_datablock(&kb_config);
      } else {
        new_setpoint_dmm = 0;
      }

      break;
    case LIGHTING:
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
