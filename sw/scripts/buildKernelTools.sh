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
echo "Error: No kernel specified"
exit 1
fi

cd $repodir

make clean -C ./tools/perf
make -C tools/perf "EXTRA_CFLAGS=-Wno-array-bounds" install prefix=${ROOT}/rootfs







