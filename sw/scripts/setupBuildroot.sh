#!/bin/bash

set -e

SCRIPT=$(realpath "$0")
SCRIPT_PATH=$(dirname "$SCRIPT")
ROOT="${SCRIPT_PATH}/.."

repodir="${ROOT}/buildroot"
rootfs="${ROOT}/rootfs"

if [ -d "$rootfs" ]
then
    sudo systemctl stop nfs-server
    sudo umount $rootfs || /bin/true
else
    mkdir $rootfs
fi

# Clone if not exists
if [ -d "$repodir" ]
then
    echo Buildroot repository found
    cd $repodir
    git reset --hard
    git clean -df 
else
    echo Cloning Buildroot..
    cd $ROOT
    git clone git@github.com:buildroot/buildroot.git
    cd $repodir
fi

git reset --hard
git clean -df 
git checkout 2023.08.4

#Apply configfile
cp $ROOT/configs/buildroot_defconfig $repodir/configs/buildroot_defconfig
make buildroot_defconfig



