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

void print_actuation_menu(void) {
  char *per_key_settings;
  char *rapid_trigger_setting;

  if (kb_config.use_per_key_settings) {
    per_key_settings = "[X]";
  } else {
    per_key_settings = "[ ]";
  }

  if (kb_config.global_actuation_settings.rapid_trigger) {
    rapid_trigger_setting = "[X]";
  } else {
    rapid_trigger_setting = "[ ]";
  }

  // Maximum actuation distance is a 2 digit integer
  char actuation_setting[2];
  sprintf(actuation_setting, "%d",
          kb_config.global_actuation_settings.actuation_point_dmm);

  char *menu_strings[] = {NL,
                          " MAIN MENU -> ACTUATION SETTINGS",
                          NL,
                          NL,
                          " [P] Per-Key Settings: ",
                          per_key_settings,
                          NL,
                          NL,
                          " [R] Rapid Trigger: ",
                          rapid_trigger_setting,
                          NL,
                          NL,
                          " [A] Actuation Distance (1 - 40): ",
                          actuation_setting,
                          NL,
                          NL,
                          " [B] BACK",
                          NL,
                          NL,
                          NULL};

  print_strings_serial(menu_strings);
}

void print_set_actuation(char *new_actuation_distance) {
  char nice[69] = {'\0'};
  if (new_actuation_distance[0] == '6' && new_actuation_distance[1] == '9') {
    strcpy(nice, "              Nice.");
  }
  char *menu_strings[] = {NL,
                          " ----------------------------------------",
                          NL,
                          NL,
                          " [C] Collapse",
                          NL,
                          NL,
                          " [Enter] Confirm",
                          nice,
                          NL,
                          NL,
                          " New Actuation Distance (1 - 40): ",
                          new_actuation_distance,
                          NULL};
  print_strings_serial(menu_strings);
  cursor_left();
}

void display_menu(enum Menu state, int actuation_distance_dmm,
                  char *new_actuation_distance) {
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
    print_set_actuation(new_actuation_distance);
    break;
  case LIGHTING:
    break;
  case RESTORE_DEFAULT:
    break;
  }
}

bool setpoint_valid(char *new_actuation_distance) {
  // single digit setpoint
  if (new_actuation_distance[1] == '\0') {
    if ('1' <= new_actuation_distance[0] && new_actuation_distance[0] <= '9') {
      return true;
    }
  }

  // double digit setpoint
  if ('1' <= new_actuation_distance[0] && new_actuation_distance[0] <= '3') {
    if ('0' <= new_actuation_distance[1] && new_actuation_distance[1] <= '9') {
      return true;
    }
  }
  if (new_actuation_distance[0] == '4' && new_actuation_distance[1] == '0') {
    return true;
  }
  return false;
}

// assumes that `new_actuation_distance` holds a valid value
int to_int(char *new_actuation_distance) {
  // single digit value
  if (new_actuation_distance[1] == '\0') {
    return new_actuation_distance[0] - '0';
  }
  int first_digit = new_actuation_distance[0] - '0';
  int second_digit = new_actuation_distance[1] - '0';
  return first_digit * 10 + second_digit;
}

void handle_menu(const uint8_t ch) {
  static enum Menu state = MAIN;
  static char new_actuation_distance[2] = {'\0'};
  int actuation_distance_dmm =
      kb_config.global_actuation_settings.actuation_point_dmm;

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
    }
    break;
  case INPUT_ACTUATION:
    if (ch == 'b' || ch == 'B') {
      state = MAIN;
    } else if (ch == 'c' || ch == 'C') {
      state = ACTUATION;
      new_actuation_distance[0] = '\0';
      new_actuation_distance[1] = '\0';
    } else if (ch == 'p' || ch == 'P') {
      kb_config.use_per_key_settings = !kb_config.use_per_key_settings;
      state = ACTUATION;
    } else if (ch == 'r' || ch == 'R') {
      kb_config.global_actuation_settings.rapid_trigger =
          !kb_config.global_actuation_settings.rapid_trigger;
      state = ACTUATION;
    } else if ('0' <= ch && ch <= '9') {
      if (new_actuation_distance[0] == '\0') {
        if ('1' <= ch && ch <= '9') {
          new_actuation_distance[0] = ch;
        }
      } else {
        // entering second digit
        new_actuation_distance[1] = ch;
      }
    } else if (ch == BS) {
      if (new_actuation_distance[1] != '\0') {
        new_actuation_distance[1] = '\0';
      } else {
        new_actuation_distance[0] = '\0';
      }
    } else if (ch == DEL) {
      new_actuation_distance[0] = '\0';
      new_actuation_distance[1] = '\0';
    } else if (ch == '\r') {
      if (setpoint_valid(new_actuation_distance)) {
        kb_config.global_actuation_settings.actuation_point_dmm =
            to_int(new_actuation_distance);
      }
      new_actuation_distance[0] = '\0';
      new_actuation_distance[1] = '\0';

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
  display_menu(state, actuation_distance_dmm, new_actuation_distance);
}

void virtser_recv(const uint8_t ch) {
  dprintf("virtser_recv: ch: %3u \n", ch);
  // TODO: handle arrow key

  handle_menu(ch);
}