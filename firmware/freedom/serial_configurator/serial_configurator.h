#pragma once

#include "../config.h"

// Bar that shows setpoints graphically
// whitespace + half of max setting + 2 encapsulation chars
// Example:
// <@@@@@@@@@@__________>
#define SETTING_BAR_SIZE 1 + (KEY_MAX_dmm / 2) + 2

#define NL "\n\r"
#define BS 8
#define DEL 127
#define ESC 27
