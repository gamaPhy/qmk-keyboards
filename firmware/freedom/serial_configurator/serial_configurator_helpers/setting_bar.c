#include "quantum.h"

#include "setting_bar.h"

// Sets the contents of `setting_bar` to a bar that fills with values
// 1 - KEY_MAX_dmm
// Ex:
// 20 <@@@@@@@@@__________>
// 40 <@@@@@@@@@@@@@@@@@@@>
//  1 <@__________________>
//
// `setting_bar` must have a size of SETTING_BAR_SIZE + 3 to fit the potentially
// 2 digit number and null termination
void create_2_digit_setting_bar(two_digit_setting_bar setting_bar,
                                int setpoint) {
  // Maximum actuation distance is a 2 digit integer pad with leading spaces if
  // there is a 1 digit integer to align setting bar
  char setting_num_str[2];
  sprintf(setting_num_str, "%2d", setpoint);

  char setting_fill[SETTING_BAR_SIZE];
  setting_fill[0] = ' ';
  setting_fill[1] = SETTING_BAR_LEFT;
  int i;
  int fill_to = 0;
  int scaled_setpoint = (setpoint * SETTING_BAR_DIVISIONS) / KEY_MAX_dmm;
  for (i = 2; i <= SETTING_BAR_DIVISIONS + 1; i++) {
    fill_to++;
    if (fill_to <= scaled_setpoint) {
      setting_fill[i] = SETTING_BAR_FILL;
    } else
      setting_fill[i] = SETTING_BAR_EMPTY;
  }
  setting_fill[i] = SETTING_BAR_RIGHT;
  setting_fill[i + 1] = '\0';
  strcpy(setting_bar, setting_num_str);
  strcat(setting_bar, setting_fill);
}

// Sets the contents of `setting_bar` to a bar that fills with values 1 - 255
// Ex:
// 127 <@@@@@@@@@__________>
// 255 <@@@@@@@@@@@@@@@@@@@>
//   1 <@__________________>
//
// `setting_bar` must have a size of SETTING_BAR_SIZE + 4 to fit the potentially
// 3 digit number and null termination
void create_3_digit_setting_bar(three_digit_setting_bar setting_bar,
                                int setpoint) {
  // Maximum value for some settings is a 3 digit number pad with leading spaces
  // if there is a 1 or 2 digit integer to align setting bar
  char setting_num_str[3];
  sprintf(setting_num_str, "%3d", setpoint);

  char setting_fill[SETTING_BAR_SIZE];
  setting_fill[0] = ' ';
  setting_fill[1] = '<';
  int i;
  int fill_to = 0;
  int scaled_setpoint = (setpoint * SETTING_BAR_DIVISIONS) / 255;
  for (i = 2; i <= SETTING_BAR_DIVISIONS + 1; i++) {
    fill_to++;
    if (fill_to <= scaled_setpoint) {
      setting_fill[i] = SETTING_BAR_FILL;
    } else
      setting_fill[i] = SETTING_BAR_EMPTY;
  }
  setting_fill[i] = SETTING_BAR_RIGHT;
  setting_fill[i + 1] = '\0';
  strcpy(setting_bar, setting_num_str);
  strcat(setting_bar, setting_fill);
}
