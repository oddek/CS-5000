
#!/bin/bash

set -e

SCRIPT=$(realpath "$0")
SCRIPT_PATH=$(dirname "$SCRIPT")
ROOT="${SCRIPT_PATH}/.."

rm -rf ${ROOT}/toolchain/crosstool-ng-1.26.0
wget -nc -P ${ROOT}/tmp http://crosstool-ng.org/download/crosstool-ng/crosstool-ng-1.26.0.tar.xz 
tar -xf ${ROOT}/tmp/crosstool-ng-1.26.0.tar.xz -C ${ROOT}/toolchain

cd ${ROOT}/toolchain/crosstool-ng-1.26.0

./bootstrap
./configure --enable-local
make

cp $ROOT/configs/crosstool-ng_defconfig configs/
./ct-ng crosstool-ng_defconfig
mkdir local-tarballs
./ct-ng build

