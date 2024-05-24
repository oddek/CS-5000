#!/bin/bash

set -e

SCRIPT=$(realpath "$0")
SCRIPT_PATH=$(dirname "$SCRIPT")
ROOT="${SCRIPT_PATH}/.."
APPDIR=${ROOT}/applications/
source $ROOT/scripts/setenv.sh

declare -a apps=("ebpf")

for f in "${apps[@]}"
do
	echo $f
	[[ -f $APPDIR/$f/Makefile ]] && echo Entering into "$f" && (cd $APPDIR/$f && make clean && make)
done