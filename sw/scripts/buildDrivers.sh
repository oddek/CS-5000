#!/bin/bash

set -e

SCRIPT=$(realpath "$0")
SCRIPT_PATH=$(dirname "$SCRIPT")
ROOT="${SCRIPT_PATH}/.."

source $ROOT/scripts/setenv.sh
shopt -s extglob

for f in ${ROOT}/drivers/**/;
do
	echo $f
	[[ -f $f/Makefile ]] && echo Entering into "$f" && (cd $f && make clean && make)
done


