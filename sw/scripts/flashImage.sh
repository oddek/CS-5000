#!/bin/bash

set -e

SCRIPT=$(realpath "$0")
SCRIPT_PATH=$(dirname "$SCRIPT")
ROOT="${SCRIPT_PATH}/.."

sdfs="${ROOT}/tmp/sdfs"

boot_script_source="${ROOT}/scripts/boot.script"

ubootspl="${ROOT}/u-boot/u-boot-with-spl.sfp"
device="/dev/mmcblk0"

zImage_rt="${ROOT}/linux-stable-rt/arch/arm/boot/zImage"
zImage_vanilla="${ROOT}/linux/arch/arm/boot/zImage"

zImage_rt_temp="${ROOT}/tmp/sdfs/zImage_rt"
zImage_vanilla_temp="${ROOT}/tmp/sdfs/zImage_vanilla"

rbf_devkit="${ROOT}/../hw/DevKit/output_files/soc_system.rbf"
dtb_devkit="${ROOT}/../hw/DevKit/output_files/soc_system.dtb"
dtb_temp="${ROOT}/tmp/sdfs/socfpga.dtb"
rbf_temp="${ROOT}/tmp/sdfs/fpga.rbf"

boot_script_out="${ROOT}/tmp/sdfs/boot.scr"
output="${ROOT}/tmp/sdimage.img"
extlinux="${ROOT}/scripts/extlinux"
extlinux_tmp="${ROOT}/tmp/sdfs/extlinux"
rootfs="${ROOT}/rootfs/*"

mkdir -p ${sdfs}
rm -rf ${sdfs}/*

echo "Flashing DevKit image"
cp ${rbf_devkit} ${rbf_temp}
cp ${dtb_devkit} ${dtb_temp}

echo $ubootspl
echo $output

cp ${zImage_rt} ${zImage_rt_temp}
cp ${zImage_vanilla} ${zImage_vanilla_temp}
cp -r ${extlinux} ${extlinux_tmp}

mkimage -c none -A arm -T script -d $boot_script_source $boot_script_out

sudo python3 ${ROOT}/scripts/make_sdimage_p3.py -f \
  -P ${ubootspl},num=3,format=raw,size=2M,type=A2 \
  -P ${rootfs},num=2,format=ext3,size=300M  \
  -P ${sdfs}/*,num=1,format=vfat,size=100M  \
  -s 500M -n ${output}

sudo dd if=${output} of=${device} bs=1M
sudo sync
