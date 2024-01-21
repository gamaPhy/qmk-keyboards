#include "quantum.h"
#include "virtser.h"

#include "serial_configurator.h"
#include "ascii_art.h"

kb_config_t kb_config;

enum Menu {
    MAIN,
    ACTUATION,
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

void cursor_home(void) {
    send_string_serial("\033[H");
}

void clear_terminal(void) {
    send_string_serial("\033[2J");
}

void reset_terminal(void) {
    clear_terminal();
    cursor_home();
}

void print_main_menu(void) {
    char* menu_strings[] = {
        "MAIN MENU",
        NL,
        "[A] Actuation Settings",
        NL,
        "[L] LED Settings",
        NL,
        NULL
    };

    print_strings_serial(menu_strings);
}

void print_actuation_menu(void) {
    bool per_key = kb_config.use_per_key_settings;
    char* per_key_settings;
        
    if (per_key) {
        per_key_settings = "TRUE";
    } else {
        per_key_settings = "FALSE";
    }

    char* menu_strings[] = {
        "MAIN MENU -> ACTUATION SETTINGS",
        NL,
        "[P] Per-Key Settings: ", per_key_settings,
        NL,
        "[B] BACK"
        NL,
        NULL
    };

    print_strings_serial(menu_strings);
}

void print_menu(enum Menu state) {
    switch (state) {
        case MAIN:
            print_main_menu();
            break;
        case ACTUATION:
            print_actuation_menu();
            break;
        case LIGHTING:
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
                state = ACTUATION;
            } 
            break;
        case ACTUATION:
            if (ch == 'b') {
                state = MAIN;
            } else if (ch == 'p' || ch == 'P') {
                kb_config.use_per_key_settings = !kb_config.use_per_key_settings;
            }
            break;
        case LIGHTING:
            if (ch == 'b') {
                state = MAIN;
            }
            break;
        case RESTORE_DEFAULT:

            break;
    }
    
    print_menu(state);
}

void virtser_recv(const uint8_t ch) {
    dprintf("virtser_recv: ch: %3u '\\n' \n", ch);
    reset_terminal();
    handle_menu(ch);
}