# Freedom

(image)

An analog osu! keypad using hall effect sensors.

* Keyboard Maintainer: [gamaPhy](https://github.com/gamaPhy)
* Hardware Supported: Freedom 3k [(open source design files)](https://github.com/gamaPhy/Freedom)
* Hardware Availability: [gamaPhy store](https://gamaphy.com/products/freedom)

Make example for this keyboard (after setting up your build environment):

    make gamaphy/freedom:default

Flashing example for this keyboard:

    make gamaphy/freedom:default:flash

See the [build environment setup](https://docs.qmk.fm/#/getting_started_build_tools) and the [make instructions](https://docs.qmk.fm/#/getting_started_make_guide) for more information. Brand new to QMK? Start with our [Complete Newbs Guide](https://docs.qmk.fm/#/newbs).

## Bootloader

Enter the bootloader in 3 ways:

* **Bootmagic reset**: Hold down the left most digital button and plug in the keyboard.
* **Physical reset button**: Press the reset button on the PCB.
* **Keycode in layout**: Press the key mapped to `QK_BOOT` if it is available.
