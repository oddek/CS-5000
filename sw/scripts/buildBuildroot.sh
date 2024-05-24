#!/bin/bash

set -e

SCRIPT=$(realpath "$0")
SCRIPT_PATH=$(dirname "$SCRIPT")
ROOT="${SCRIPT_PATH}/.."

source $ROOT/scripts/setenv.sh

cd $ROOT/buildroot

# Save current config
make savedefconfig
cp configs/buildroot_defconfig ../configs/

sudo systemctl stop nfs-server
sudo umount ../rootfs || /bin/true
make -j8

$SCRIPT_PATH/remountRootfs.sh





