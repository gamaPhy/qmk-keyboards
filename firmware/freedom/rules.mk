VIRTSER_ENABLE = yes
EXTRAKEY_ENABLE = yes
WS2812_DRIVER = vendor
EEPROM_DRIVER = wear_leveling
WEAR_LEVELING_DRIVER = rp2040_flash
# WEAR_LEVELING_DRIVER = legacy

CONSOLE_ENABLE = yes
DEFAULT_FOLDER = gamaphy/freedom/3k

RGBLIGHT_ENABLE = yes
RGBLIGHT_DRIVER = ws2812
# WS2812_DRIVER = pwm

SRC += matrix.c
SRC += analog.c
SRC += sensor_read.c
