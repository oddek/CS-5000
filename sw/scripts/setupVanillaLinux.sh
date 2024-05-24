#!/bin/bash

set -e

SCRIPT=$(realpath "$0")
SCRIPT_PATH=$(dirname "$SCRIPT")
ROOT="${SCRIPT_PATH}/.."

source $ROOT/scripts/setenv.sh

repodir="${ROOT}/linux"
config_source_dir="${ROOT}/configs"
configfile="linux_defconfig"
config_dest="${repodir}/arch/arm/configs"
tag="v6.6.14"

# Clone if not exists
if [ -d "$repodir" ]
then
    echo Linux repository found
else
    echo Cloning Linux..
    cd $ROOT
    git clone git://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git
fi

cd $repodir
git reset --hard
git clean -df 
git checkout $tag

#Apply configfile
cp $config_source_dir/$configfile $config_dest
make $configfile

