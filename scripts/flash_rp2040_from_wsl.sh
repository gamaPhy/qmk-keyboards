#!/bin/bash

# set directory to script file's locations
cd "$(dirname "$0")"
cd ../
./in_qmk.sh qmk compile -kb gamaphy/freedom/3k -km via -e TEST=1
sudo mount -t drvfs F: /mnt/f
cp qmk_firmware/gamaphy_freedom_3k_via.uf2 /mnt/f/
