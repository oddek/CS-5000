#!/bin/bash

set -e

SCRIPT=$(realpath "$0")
SCRIPT_PATH=$(dirname "$SCRIPT")
ROOT="${SCRIPT_PATH}/.."

source $ROOT/scripts/setenv.sh

if [ "$1" == "rt" ]; then
repodir="${ROOT}/linux-stable-rt"
configfile="linuxrt_defconfig"
elif [ "$1" == "vanilla" ]; then
repodir="${ROOT}/linux"
configfile="linux_defconfig"
elif [ "$1" == "vanilla-dynamic" ]; then
repodir="${ROOT}/linux-preempt-dynamic"
configfile="linux-preempt-dynamic_defconfig"
else
exit 1
fi

config_source_dir="${ROOT}/configs"
config_dest="${repodir}/arch/arm/configs"

cd $repodir

# Save current config
make savedefconfig
cp defconfig $config_source_dir/$configfile
cp $config_source_dir/$configfile $config_dest

make zImage modules -j$(nproc)

sudo cp arch/arm/boot/zImage /srv/tftp/zImage
sudo make INSTALL_MOD_PATH=$ROOT/rootfs modules_install

sudo make INSTALL_HDR_PATH=$ROOT/rootfs headers_install

bpftool btf dump file ./vmlinux format c > ./vmlinux.h

cd $SCRIPT_PATH

./buildDrivers.sh $1




