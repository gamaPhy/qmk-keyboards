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
                          " [A] Actuation Settings",
                          NL,
                          NL,
                          " [L] LED Settings",
                          NL,
                          NL,
                          NULL};

  print_strings_serial(menu_strings);
}

void create_setting_bar(char *setting_bar, int setpoint) {
  setting_bar[0] = '[';
  int i;
  for (i = 1; i <= KEY_MAX_dmm; i++) {
    if (i <= setpoint) {
      setting_bar[i] = 'X';
    } else
      setting_bar[i] = '-';
  }
  setting_bar[i] = ']';
  setting_bar[i + 1] = '\0';
}

void print_actuation_menu(void) {
  char *per_key_settings;
  char *rapid_trigger_setting;

  if (kb_config.use_per_key_settings) {
    per_key_settings = "[X]";
  } else {
    per_key_settings = "[ ]";
  }

  if (kb_config.global_actuation_settings.rapid_trigger) {
    rapid_trigger_setting = "   [X]";
  } else {
    rapid_trigger_setting = "   [ ]";
  }

  // Maximum actuation distance is a 2 digit integer
  // pad with leading spaces if there is a 1 digit integer to align setting bars
  char actuation_setting[2];
  sprintf(actuation_setting, "%2d",
          kb_config.global_actuation_settings.actuation_point_dmm);
  char actuation_setting_bar[SETTING_BAR_SIZE];
  create_setting_bar(actuation_setting_bar,
                     kb_config.global_actuation_settings.actuation_point_dmm);

  char press_sensitivity_setting[2];
  sprintf(
      press_sensitivity_setting, "%2d",
      kb_config.global_actuation_settings.rapid_trigger_press_sensitivity_dmm);
  char press_setting_bar[SETTING_BAR_SIZE];
  create_setting_bar(
      press_setting_bar,
      kb_config.global_actuation_settings.rapid_trigger_press_sensitivity_dmm);

  char release_sensitivity_setting[2];
  sprintf(release_sensitivity_setting, "%2d",
          kb_config.global_actuation_settings
              .rapid_trigger_release_sensitivity_dmm);
  char release_setting_bar[SETTING_BAR_SIZE];
  create_setting_bar(release_setting_bar,
                     kb_config.global_actuation_settings
                         .rapid_trigger_release_sensitivity_dmm);

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
                          actuation_setting,
                          " ",
                          actuation_setting_bar,
                          NL,
                          NL,
                          " [E] Press Sensitivity   ",
                          press_sensitivity_setting,
                          " ",
                          press_setting_bar,
                          NL,
                          NL,
                          " [L] Release Sensitivity ",
                          release_sensitivity_setting,
                          " ",
                          release_setting_bar,
                          NL,
                          NL,
                          " [B] Back",
                          NL,
                          NULL};

  print_strings_serial(menu_strings);
}

void print_set_new_setpoint(char *prompt, int new_setpoint_dmm) {
  char nice[69] = {'\0'};
  if (new_setpoint_dmm == 69) {
    strcpy(nice, "              Nice.");
  }
  char new_setpoint_str[2] = {'\0'};
  if (new_setpoint_dmm != 0) {
    sprintf(new_setpoint_str, "%d", new_setpoint_dmm);
  }

  char *menu_strings[] = {NL,
                          " ---------------------------------------------------"
                          "-------------------------",
                          NL,
                          NL,
                          " [I] Increase",
                          NL,
                          NL,
                          " [D] Decrease",
                          NL,
                          NL,
                          " [Enter] Confirm",
                          NL,
                          NL,
                          " [X] Cancel",
                          nice,
                          NL,
                          NL,
                          prompt,
                          new_setpoint_str,
                          NULL};
  print_strings_serial(menu_strings);
  cursor_left();
}

void display_menu(enum Menu state, int new_setpoint_dmm) {
  reset_terminal();

  switch (state) {
  case MAIN:
    print_main_menu();
    break;
  case ACTUATION:
    print_actuation_menu();
    break;
  case INPUT_ACTUATION:
    print_actuation_menu();
    print_set_new_setpoint(" New Actuation Distance (1 - 40): ",
                           new_setpoint_dmm);
    break;
  case INPUT_PRESS_SENSITIVITY:
    print_actuation_menu();
    print_set_new_setpoint(" New Press Sensitivity (1 - 40): ",
                           new_setpoint_dmm);
    break;
  case INPUT_RELEASE_SENSITIVITY:
    print_actuation_menu();
    print_set_new_setpoint(" New Release Sensitivity (1 - 40): ",
                           new_setpoint_dmm);
    break;
  case LIGHTING:
    break;
  case RESTORE_DEFAULT:
    break;
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
    }
    break;
  case ACTUATION:
    if (ch == 'b' || ch == 'B') {
      state = MAIN;
    } else if (ch == 'p' || ch == 'P') {
      kb_config.use_per_key_settings = !kb_config.use_per_key_settings;
    } else if (ch == 'r' || ch == 'R') {
      kb_config.global_actuation_settings.rapid_trigger =
          !kb_config.global_actuation_settings.rapid_trigger;
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
      state = ACTUATION;
    } else if (ch == 'r' || ch == 'R') {
      kb_config.global_actuation_settings.rapid_trigger =
          !kb_config.global_actuation_settings.rapid_trigger;
      state = ACTUATION;
    } else if (ch == 'a' || ch == 'A') {
      state = INPUT_ACTUATION;
    } else if (ch == 'e' || ch == 'E') {
      state = INPUT_PRESS_SENSITIVITY;
    } else if (ch == 'l' || ch == 'L') {
      state = INPUT_RELEASE_SENSITIVITY;
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
    } else if (ch == ARROW_UP) {

    } else if (ch == BS) {
      if (new_setpoint_dmm >= 10) {
        new_setpoint_dmm = new_setpoint_dmm / 10;
      } else {
        new_setpoint_dmm = 0;
      }
    } else if (ch == DEL) {
      new_setpoint_dmm = 0;
    } else if (ch == '\r') {
      // 'Enter' pressed
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
      }
      new_setpoint_dmm = 0;

      break;
    case LIGHTING:
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
