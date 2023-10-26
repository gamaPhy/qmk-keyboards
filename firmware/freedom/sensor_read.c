#include "sensor_read.h"

int sensorRead(pin_t pin) {
  if (POLARITY_FLIP) {
    return MAX_ADC_READING - analogReadPin(pin);
  } else {
    return analogReadPin(pin);
  }
}

int oversample(pin_t pin) {
    uint32_t total = 0;
    for (int i = 0; i < OVERSAMPLING_TOTAL_SAMPLES; i++) {
        total += sensorRead(pin);
    }
    
    return total / OVERSAMPLING_TOTAL_SAMPLES;
}