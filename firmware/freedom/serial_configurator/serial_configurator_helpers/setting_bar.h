#pragma once

// Denotes which sliding-bar setting is currently selected to be changed
#define SELECT_PREFIX " --> ";
#define SELECT_SUFFIX " <-- ";

// "Sliding-bar" that shows setpoints graphically
// Example:
// <@@@@@@@@@@__________>
#define SETTING_BAR_LEFT '<'
#define SETTING_BAR_RIGHT '>'
#define SETTING_BAR_FILL '@'
#define SETTING_BAR_EMPTY '_'
#define SETTING_BAR_DIVISIONS 20
// leading whitespace + number of divisions + 2 encapsulation chars
#define SETTING_BAR_SIZE 1 + SETTING_BAR_DIVISIONS + 2

// digits + null termination
typedef char two_digit_setting_bar[SETTING_BAR_SIZE + 3];
typedef char three_digit_setting_bar[SETTING_BAR_SIZE + 4];

void create_2_digit_setting_bar(two_digit_setting_bar buffer, int setpoint);
void create_3_digit_setting_bar(three_digit_setting_bar buffer, int setpoint);
