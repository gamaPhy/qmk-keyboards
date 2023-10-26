#include "sensor_read.h"

int sensorRead(pin_t pin) {
  if (POLARITY_FLIP) {
    return MAX_ADC_READING - analogReadPin(pin);
  } else {
    return analogReadPin(pin);
  }
}

uint32_t oversample(pin_t pin, int samples) {
    uint32_t total = 0;
    for (int i = 0; i < samples; i++) {
        total += sensorRead(pin);
    }
    
    return total / samples;
}