// Copyright 2023 gamaPhy (https://gamaphy.com/)
// SPDX-License-Identifier: GPL-3.0-or-later
#include QMK_KEYBOARD_H

enum layers {
    _OSU = 0,
    _SET
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_OSU] = LAYOUT(
        KC_Z,   KC_X,         KC_C,
        QK_BOOT, KC_CALIBRATE, KC_SPACE
        KC_ESC, MO(1), KC_CALIBRATE
    ),
    [1] = LAYOUT(
     // RGB_VAD = Brightness decrease
     // RGB_VAI = Brightness increase
        KC_BSPC,   RGB_VAD,   RGB_VAI,
        QK_BOOT, KC_NO, RGB_TOG
    )
};
