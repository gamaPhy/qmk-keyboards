VIRTSER_ENABLE = yes
EXTRAKEY_ENABLE = yes
WS2812_DRIVER = vendor
EEPROM_DRIVER = wear_leveling
WEAR_LEVELING_DRIVER = rp2040_flash
# WEAR_LEVELING_DRIVER = legacy

CONSOLE_ENABLE = yes
DEFAULT_FOLDER = gamaphy/freedom/3k

RGB_MATRIX_ENABLE = yes
RGB_MATRIX_DRIVER = ws2812
### this is for STM MCUs, rp2040 uses PIO, which is already defined in config.h
# WS2812_DRIVER = pwm

SRC += matrix.c
SRC += analog.c
SRC += helpers/sensor_read.c
SRC += helpers/lookup_table.c
SRC += helpers/kb_config.c
SRC += serial_configurator/serial_configurator.c
SRC += serial_configurator/serial_configurator_helpers/setting_bar.c
