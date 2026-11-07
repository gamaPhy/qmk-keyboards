#!/bin/bash

# Check if the drive letter argument is provided
if [ -z "$1" ]; then
  echo "Usage: $0 <drive_letter>"
  exit 1
fi

# Convert the drive letter to uppercase if it isn't already
drive_letter_upper=$(echo "$1" | tr '[:lower:]' '[:upper:]')

# Convert the drive letter to lowercase if it isn't already
drive_letter_lower=$(echo "$1" | tr '[:upper:]' '[:lower:]')

# set directory to script file's locations
cd "$(dirname "$0")"
cd ../
./in_qmk.sh qmk compile -kb gamaphy/freedom/3k -km via -e TEST=1
sudo mount -t drvfs "$drive_letter_upper": /mnt/"$drive_letter_lower"
cp qmk_firmware/gamaphy_freedom_3k_via.uf2 /mnt/"$drive_letter_lower"/
