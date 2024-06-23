#pragma once

#include "../config.h"
#include "../freedom.h"

// Bar that shows setpoints graphically
// 40 total setpoints + 2 encapsulation chars
#define SETTING_BAR_SIZE KEY_MAX_dmm + 2

#define NL "\n\r"
#define BS 8
#define DEL 127
#define ESC 27

// Define arrow key characters as the last ascii 8-bit addresses
#define ARROW_UP (1 << 8) - 1    // esc[A
#define ARROW_DOWN (1 << 8) - 2  // esc[B
#define ARROW_RIGHT (1 << 8) - 4 // esc[C
#define ARROW_LEFT (1 << 8) - 3  // esc[D
