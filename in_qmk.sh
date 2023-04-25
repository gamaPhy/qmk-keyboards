#!/bin/bash
mv qmk_firmware/keyboards/gamaphy qmk_firmware/keyboards/gamaphy.original
gamaphy_dir_already_exist=$?
mkdir qmk_firmware/keyboards/gamaphy

ln -s "$PWD/firmware/"* qmk_firmware/keyboards/gamaphy || exit
cd qmk_firmware || exit
# Run the rest of the command
${@:1}
rm -r keyboards/gamaphy

if [ $gamaphy_dir_already_exist -eq 0 ]; then
    mv keyboards/gamaphy.original keyboards/gamaphy
fi