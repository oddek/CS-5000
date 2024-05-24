#!/bin/bash

set -e

SCRIPT=$(realpath "$0")
SCRIPT_PATH=$(dirname "$SCRIPT")
ROOT="${SCRIPT_PATH}/.."

ubootspl="${ROOT}/u-boot/u-boot-with-spl.sfp"
device="/dev/mmcblk0"

echo "Flashing DevKit image"
boot_script_source="${ROOT}/scripts/netboot-devkit.script"

boot_script_out="${ROOT}/tmp/boot.scr"
output="${ROOT}/tmp/sdimage.img"
rootfs="${ROOT}/configs/*"

echo $ubootspl
echo $output

mkimage -c none -A arm -T script -d $boot_script_source $boot_script_out

sudo python3 ${ROOT}/scripts/make_sdimage_p3.py -f \
  -P ${ubootspl},num=3,format=raw,size=2M,type=A2 \
  -P ${rootfs},num=2,format=ext3,size=60M  \
  -P ${boot_script_out},num=1,format=vfat,size=1M  \
  -s 100M -n ${output}

sudo dd if=${output} of=${device} bs=1M
sudo sync
