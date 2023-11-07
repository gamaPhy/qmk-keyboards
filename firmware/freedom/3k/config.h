// Copyright 2023 gamaPhy (https://gamaphy.com/)
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

// row and col to mimic bootmagic functionality so that sensor calibration data is not wiped
#define BOOT_ROW 1
#define BOOT_COL 0

// Set to maximum number which still results in matrix scan frequency greater than 1000
// since this means at least 1ms polling frequency
#define OVERSAMPLING_TOTAL_SAMPLES 37

#define PIN_SCAN_MODES { \
    { ANALOG,  ANALOG,  ANALOG }, \
    { DIGITAL, DIGITAL, DIGITAL } \
}

// -1 means there is no analog sensor 
#define SENSOR_NUM { \
    { 0,  1,  2 }, \
    { -1, -1, -1 } \
}

// total hall effect sensors
#define SENSOR_COUNT 3
