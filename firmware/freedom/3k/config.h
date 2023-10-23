// Copyright 2023 gamaPhy (https://gamaphy.com/)
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

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
