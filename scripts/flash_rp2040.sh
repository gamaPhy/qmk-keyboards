#!/bin/bash

if [ -z "$1" ]; then
  echo "Usage: $0 <rpi_path>"
fi

# Get the provided path
mount_path="$1"

# Set the directory to the script file's location
cd "$(dirname "$0")"
cd ../

# Compile the firmware
./in_qmk.sh qmk compile -kb gamaphy/freedom/3k -km via -e TEST=1

# Copy the file to the provided mount path
cp qmk_firmware/gamaphy_freedom_3k_via.uf2 "$mount_path"/

# Check if the provided path exists
if [ ! -d "$mount_path" ]; then
  echo "Error: The provided path '$mount_path' does not exist."
fi
