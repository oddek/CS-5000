#!/bin/bash

set -e

SCRIPT=$(realpath "$0")
SCRIPT_PATH=$(dirname "$SCRIPT")
ROOT="${SCRIPT_PATH}/.."

ubootDir="${ROOT}/u-boot"

# Clone if not exists
if [ -d "$ubootDir" ]
then
    echo U-Boot repository found
    cd $ubootDir
else
    echo Cloning U-Boot..
    cd $ROOT
    git clone git@github.com:u-boot/u-boot.git
    cd u-boot
    git checkout v2023.07
fi



