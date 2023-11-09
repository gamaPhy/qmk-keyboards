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
        KC_ESC, RGB_TOG, KC_CALIBRATE
    )
};
