# qmk-keyboards

This repository contains the code and binaries for our keypads/keyboards based on [QMK](https://github.com/qmk/qmk_firmware).

## Latest Firmware

The most recent firmware for every keypad/keyboard is in the [latest release](https://github.com/gamaPhy/qmk-keyboards/releases/latest) of this repository.

## Development Usage

The `in_qmk.sh` script takes any command as an argument, and then runs that command in a QMK environment that has access to the keyboards in `firmware/`. These keyboards be inside the `gamaphy` directory in the QMK environment. So to build firmware for Freedom (3k) that suports VIA, you could run:

    ./in_qmk.sh qmk compile -kb gamaphy/freedom/3k -km via

And to flash, you could run:

    ./in_qmk.sh qmk flash -kb gamaphy/freedom/3k -km via

I suggest using the `qmk` commands instead of `make` commands, since interrupting a `make` command will mess up the files in the `qmk_firmware` submodule.
