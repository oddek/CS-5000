#!/bin/bash


SCRIPT=$(realpath "$0")
SCRIPT_PATH=$(dirname "$SCRIPT")
ROOT="${SCRIPT_PATH}/.."

export ARCH=arm
export CROSS_COMPILE=${ROOT}/toolchain/x-tools/arm-cortexa9_neon-linux-gnueabihf/bin/arm-cortexa9_neon-linux-gnueabihf-

if [ -n "$1" ]; then
    if [ "$1" == "vanilla" ]; then
        export KDIR=${ROOT}/linux
        export LKM_INSTALL_PATH=${ROOT}/rootfs/lib/modules/6.6.14/kernel/drivers
    elif [ "$1" == "vanilla-dynamic" ]; then
        export KDIR=${ROOT}/linux-preempt-dynamic
        export LKM_INSTALL_PATH=${ROOT}/rootfs/lib/modules/6.6.14-preempt-dynamic/kernel/drivers
    elif [ "$1" == "rt" ]; then
        export KDIR=${ROOT}/linux-stable-rt
        export LKM_INSTALL_PATH=${ROOT}/rootfs/lib/modules/6.6.14-rt21/kernel/drivers
    else
        exit 1
    fi
else
    export KDIR=${ROOT}/linux-stable-rt
    export LKM_INSTALL_PATH=${ROOT}/rootfs/lib/modules/6.1.38-rt12/kernel/drivers
fi

export ROOTFS=${ROOT}/rootfs/
export ROOTFS_HOME=${ROOT}/rootfs/root

echo ARCH: $ARCH
echo CROSS_COMPILE: $CROSS_COMPILE
echo KDIR: $KDIR
echo ROOTFS: $ROOTFS
echo ROOTFS_HOME: $ROOTFS_HOME
echo LKM_INSTALL_PATH: $LKM_INSTALL_PATH
