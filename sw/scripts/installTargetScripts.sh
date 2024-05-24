#!/bin/sh

SCRIPT=$(realpath "$0")
SCRIPT_PATH=$(dirname "$SCRIPT")
ROOT="${SCRIPT_PATH}/.."

sudo cp ${ROOT}/target-scripts/* ${ROOT}/rootfs/root/