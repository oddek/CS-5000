#!/bin/bash

SCRIPT=$(realpath "$0")
SCRIPT_PATH=$(dirname "$SCRIPT")
ROOT="${SCRIPT_PATH}/.."

source $ROOT/scripts/setenv.sh

if [ "$1" == "rt" ]; then
repodir="${ROOT}/linux-stable-rt"
elif [ "$1" == "vanilla" ]; then
repodir="${ROOT}/linux"
elif [ "$1" == "vanilla-dynamic" ]; then
repodir="${ROOT}/linux-preempt-dynamic"
else
exit 1
fi

cd $repodir

sudo cp arch/arm/boot/zImage /srv/tftp/zImage
sudo make INSTALL_MOD_PATH=$ROOT/rootfs modules_install

sudo make INSTALL_HDR_PATH=$ROOT/rootfs headers_install

cd $SCRIPT_PATH

./buildDrivers.sh $1