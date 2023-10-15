# qmk-keyboards

This repository contains the code and binaries for our keypads/keyboards based on [QMK](https://github.com/qmk/qmk_firmware).

## Latest Firmware

The most recent firmware for every keypad/keyboard is in the [latest release](https://github.com/gamaPhy/qmk-keyboards/releases/latest) of this repository.

## Development Usage

### Getting Started
`git clone https://github.com/gamaPhy/qmk-keyboards.git --recurse-submodules`

### Workflow
The `in_qmk.sh` script takes any command as an argument, and then runs that command in a QMK environment that has access to the keyboards in `firmware/`. These keyboards are inside the `gamaphy` directory in the QMK environment. So to build firmware for Freedom (3k) that supports VIA, you could run:

    ./in_qmk.sh qmk compile -kb gamaphy/freedom/3k -km via

And to flash, you could run:

    ./in_qmk.sh qmk flash -kb gamaphy/freedom/3k -km via

It is suggested to use the `qmk` commands instead of `make` commands, since interrupting a `make` command will mess up the files in the `qmk_firmware` submodule.

## Implementation

### Sensor scaling and calibration

The output of a HE(Hall Effect) sensor, `sX_out` (where `X` is the sensor number), is an analog voltage value that is read by the ADC. The calibration and scaling process maps each value of `sX_out` to the distance that the switch `X` is being pressed. Each sensor has its own lookup table, which is stored as an array in static memory. All the lookup tables are loaded into RAM when the board boots up.

