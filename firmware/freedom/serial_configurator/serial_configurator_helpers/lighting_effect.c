#include "quantum.h"

#include "lighting_effect.h"

char *lighting_mode_string(void) {
  switch (rgb_matrix_get_mode()) {
  case RGB_MATRIX_NONE:
    return "None";
  case RGB_MATRIX_SOLID_COLOR:
    return "Solid Color";
  case RGB_MATRIX_ALPHAS_MODS:
    return "Alphas Mods";
  case RGB_MATRIX_GRADIENT_UP_DOWN:
    return "Gradient Up Down";
  case RGB_MATRIX_GRADIENT_LEFT_RIGHT:
    return "Gradient Left Right";
  case RGB_MATRIX_BREATHING:
    return "Breathing";
  case RGB_MATRIX_BAND_SAT:
    return "Band Saturation";
  case RGB_MATRIX_BAND_VAL:
    return "Band Brightness";
  case RGB_MATRIX_BAND_PINWHEEL_SAT:
    return "Band Pinwheel Sat";
  case RGB_MATRIX_BAND_PINWHEEL_VAL:
    return "Band Pinwheel Val";
  case RGB_MATRIX_BAND_SPIRAL_SAT:
    return "Band Spiral Sat";
  case RGB_MATRIX_BAND_SPIRAL_VAL:
    return "Band Spiral Val";
  case RGB_MATRIX_CYCLE_ALL:
    return "Cycle All";
  case RGB_MATRIX_CYCLE_LEFT_RIGHT:
    return "Cycle Left Right";
  case RGB_MATRIX_CYCLE_UP_DOWN:
    return "Cycle Up Down";
  case RGB_MATRIX_CYCLE_OUT_IN:
    return "Cycle Out In";
  case RGB_MATRIX_CYCLE_OUT_IN_DUAL:
    return "Cycle Out In Dual";
  case RGB_MATRIX_RAINBOW_MOVING_CHEVRON:
    return "Rainbow Moving Chevron";
  case RGB_MATRIX_CYCLE_PINWHEEL:
    return "Cycle Pinwheel";
  case RGB_MATRIX_CYCLE_SPIRAL:
    return "Cycle Spiral";
  case RGB_MATRIX_DUAL_BEACON:
    return "Dual Beacon";
  case RGB_MATRIX_RAINBOW_BEACON:
    return "Rainbow Beacon";
  case RGB_MATRIX_RAINBOW_PINWHEELS:
    return "Rainbow Pinwheels";
  case RGB_MATRIX_FLOWER_BLOOMING:
    return "Flower Blooming";
  case RGB_MATRIX_RAINDROPS:
    return "Raindrops";
  case RGB_MATRIX_JELLYBEAN_RAINDROPS:
    return "Jellybean Raindrops";
  case RGB_MATRIX_HUE_BREATHING:
    return "Hue Breathing";
  case RGB_MATRIX_HUE_PENDULUM:
    return "Hue Pendulum";
  case RGB_MATRIX_HUE_WAVE:
    return "Hue Wave";
  case RGB_MATRIX_PIXEL_FRACTAL:
    return "Pixel Fractal";
  case RGB_MATRIX_PIXEL_FLOW:
    return "Pixel Flow";
  case RGB_MATRIX_PIXEL_RAIN:
    return "Pixel Rain";
  case RGB_MATRIX_TYPING_HEATMAP:
    return "Typing Heatmap";
  case RGB_MATRIX_DIGITAL_RAIN:
    return "Digital Rain";
  case RGB_MATRIX_SOLID_REACTIVE_SIMPLE:
    return "Solid Reactive Simple";
  case RGB_MATRIX_SOLID_REACTIVE:
    return "Solid Reactive";
  case RGB_MATRIX_SOLID_REACTIVE_WIDE:
    return "Solid Reactive Wide";
  case RGB_MATRIX_SOLID_REACTIVE_MULTIWIDE:
    return "Solid Reactive Multiwide";
  case RGB_MATRIX_SOLID_REACTIVE_CROSS:
    return "Solid Reactive Cross";
  case RGB_MATRIX_SOLID_REACTIVE_MULTICROSS:
    return "Solid Reactive Multicross";
  case RGB_MATRIX_SOLID_REACTIVE_NEXUS:
    return "Solid Reactive Nexus";
  case RGB_MATRIX_SOLID_REACTIVE_MULTINEXUS:
    return "Solid Reactive Multinexus";
  case RGB_MATRIX_SPLASH:
    return "Splash";
  case RGB_MATRIX_MULTISPLASH:
    return "Multisplash";
  case RGB_MATRIX_SOLID_SPLASH:
    return "Solid Splash";
  case RGB_MATRIX_SOLID_MULTISPLASH:
    return "Solid Multisplash";
  case RGB_MATRIX_STARLIGHT:
    return "Starlight";
  case RGB_MATRIX_STARLIGHT_DUAL_HUE:
    return "Starlight Dual Hue";
  case RGB_MATRIX_STARLIGHT_DUAL_SAT:
    return "Starlight Dual Sat";
  case RGB_MATRIX_RIVERFLOW:
    return "Riverflow";
  case RGB_MATRIX_EFFECT_MAX:
    return "Effect Max";
  default:
    return "None";
  }
}
