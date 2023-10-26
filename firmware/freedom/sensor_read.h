#pragma once

#include "quantum.h"
#include "analog.h"

// If the voltage output of the sensor increases as the switch is pressed further, POLARITY_FLIP is false.
// otherwise, POLARITY_FLIP is true.
#define POLARITY_FLIP true

int sensorRead(pin_t pin);

int oversample(pin_t pin);