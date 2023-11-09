// Copyright 2023 gamaPhy (https://gamaphy.com/)
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#define BOOTMAGIC_LITE_ROW 1
#define BOOTMAGIC_LITE_COLUMN 0

// Set to maximum number which still results in matrix scan frequency greater than 500
// since this means at most 2ms matrix scan frequency
#define OVERSAMPLING_TOTAL_SAMPLES 70

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
