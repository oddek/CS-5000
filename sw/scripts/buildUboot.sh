#!/bin/bash

set -e

SCRIPT=$(realpath "$0")
SCRIPT_PATH=$(dirname "$SCRIPT")
ROOT="${SCRIPT_PATH}/.."

source $ROOT/scripts/setenv.sh

cd $ROOT/u-boot
echo $PWD

#Clean
make distclean
git restore ./board/altera/cyclone5-socdk/
git restore ./include/

# Load Config
make socfpga_cyclone5_defconfig

echo "Build U-Boot for DevKit"
patch -p1 < $ROOT/configs/uboot202307-devkit.patch
make -j8

#Manual:
# setenv ethaddr 8a:c3:9b:10:29:39;
# setenv bootcmd 'run distro_bootcmd';
# saveenv;
# reset;