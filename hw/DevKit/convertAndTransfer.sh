#!/bin/bash

set -e

sof="./output_files/soc_system.sof"
flash="./output_files/soc_system.flash"
rbf="./output_files/soc_system.rbf"
dtb="./output_files/soc_system.dtb"
dts="./output_files/soc_system.dts"
header="./output_files/soc_system.h"

tftp_dir="/srv/tftp/"
rbf_dest="fpga-devkit.rbf"
dtb_dest="socfpga-devkit.dtb"

sof2flash --offset=0 --input=$sof --output=$flash
nios2-elf-objcopy -I srec -O binary $flash $rbf

sopc2dts --input "./soc_system.sopcinfo"\
  --output $dtb\
  --type dtb\
  --board "soc_system_board_info.xml"\
  --board "hps_common_board_info.xml"\
  --bridge-removal all\
  --clocks \
  -v 
  sopc2dts --input "./soc_system.sopcinfo"\
  --output $dts\
  --type dts\
  --board "soc_system_board_info.xml"\
  --board "hps_common_board_info.xml"\
  --bridge-removal all\
  --clocks

sudo cp $rbf $tftp_dir/$rbf_dest
sudo cp $dtb $tftp_dir/$dtb_dest


  
