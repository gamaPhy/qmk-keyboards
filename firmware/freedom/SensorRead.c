#include "SensorRead.h"

int sensorRead(pin_t pin) {
  if (POLARITY_FLIP) {
    return MAX_ADC_READING - analogReadPin(pin);
  } else {
    return analogReadPin(pin);
  }
}