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

void print_strings_serial(char *lines[]) {
  int i;
  while (lines[i] != NULL) {
    send_string_serial(lines[i]);
    i++;
  }
}

void cursor_home(void) { send_string_serial("\033[H"); }

void cursor_up(void) { send_string_serial("\033[1A"); }

void cursor_down(void) { send_string_serial("\033[1B"); }

void cursor_right(int cols) {
  char cols_string[5];
  sprintf(cols_string, "\033[%dC", cols);
  send_string_serial(cols_string);
}

void clear_terminal(void) { send_string_serial("\033[2J"); }

void reset_terminal(void) {
  clear_terminal();
  cursor_home();
}

void print_main_menu(void) {
  char *menu_strings[] = {
      NL, "MAIN MENU",        NL, NL,  "[A] Actuation Settings",
      NL, "[L] LED Settings", NL, NULL};

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
                          "MAIN MENU -> ACTUATION SETTINGS",
                          NL,
                          NL,
                          "[P] Per-Key Settings: ",
                          per_key_settings,
                          NL,
                          "[R] Rapid Trigger: ",
                          rapid_trigger_setting,
                          NL,
                          "[A] Actuation Point (1 - 40): ",
                          actuation_setting,
                          NL,
                          "[B] BACK",
                          NL,
                          NULL};

  print_strings_serial(menu_strings);
}

void print_set_actuation(int actuation_setpoint_dmm) { 
  char actuation_setting[2];
  sprintf(actuation_setting, "%d",
          actuation_setpoint_dmm);

  char *menu_strings[] = {
      NL, 
      "[C] Cancel",
      NL, 
      "New actuation point (1 - 40): ",
      actuation_setting,
      NULL};
  print_strings_serial(menu_strings); 
}

void display_menu(enum Menu state, int actuation_setpoint_dmm) {
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
    print_set_actuation(actuation_setpoint_dmm);
    break;
  case LIGHTING:
    break;
  case RESTORE_DEFAULT:
    break;
  }
}

void handle_menu(const uint8_t ch) {
  static enum Menu state = MAIN;
  int actuation_setpoint_dmm = kb_config.global_actuation_settings.actuation_point_dmm;

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
    } else if (ch == 'p' || ch == 'P') {
      kb_config.use_per_key_settings = !kb_config.use_per_key_settings;
      state = ACTUATION;
    } else if (ch == 'r' || ch == 'R') {
      kb_config.global_actuation_settings.rapid_trigger =
          !kb_config.global_actuation_settings.rapid_trigger;
      state = ACTUATION;
    } else if ('0' <= ch && ch <= '9') {
        actuation_setpoint_dmm = ch - '0';
    }
    break;
  case LIGHTING:
    if (ch == 'b' || ch == 'B') {
      state = MAIN;
    }
    break;
  case RESTORE_DEFAULT:

    break;
  }

  display_menu(state, actuation_setpoint_dmm);
}

void virtser_recv(const uint8_t ch) {
  dprintf("virtser_recv: ch: %3u '\\n' \n", ch);
  handle_menu(ch);
}
