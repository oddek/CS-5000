#!/bin/bash

set -e

SCRIPT=$(realpath "$0")
SCRIPT_PATH=$(dirname "$SCRIPT")
ROOT="${SCRIPT_PATH}/.."
linux_rt_dir=${ROOT}/linux-stable-rt
linux_vanilla_dir=${ROOT}/linux
buildroot_image=${ROOT}/buildroot/output/images/rootfs.ext3

source $ROOT/scripts/setenv.sh

sudo systemctl stop nfs-server
sudo umount ../rootfs || /bin/true

sudo mount $buildroot_image ../rootfs 

sudo chmod 777 ../rootfs
sudo chmod 777 ../rootfs/root
sudo systemctl start nfs-server

#install modules
cd $linux_rt_dir
sudo make INSTALL_MOD_PATH=$ROOT/rootfs modules_install
cd $linux_vanilla_dir
sudo make INSTALL_MOD_PATH=$ROOT/rootfs modules_install

#install out of tree modules
cd $SCRIPT_PATH
$SCRIPT_PATH/buildDrivers.sh rt
$SCRIPT_PATH/buildDrivers.sh vanilla
$SCRIPT_PATH/buildSut.sh
$SCRIPT_PATH/buildApplications.sh rt
$SCRIPT_PATH/installTargetScripts.sh
# $SCRIPT_PATH/buildKernelTools.sh rt





