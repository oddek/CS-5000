#!/bin/sh
build=$(uname -r)

./load_generator

filename=alloc_${build}.res
sleeptime=5

echo "Running alloctest on $build, storing to $filename"  
echo "Running alloctest on $build" > $filename
echo " " >> $filename

./alloc.sh memlock >> $filename 2>&1
sleep $sleeptime
./alloc.sh dont-memlock >> $filename 2>&1
sleep $sleeptime
./alloc.sh memlock stress-ng 25 >> $filename 2>&1
sleep $sleeptime
./alloc.sh dont-memlock stress-ng 25 >> $filename 2>&1
sleep $sleeptime
./alloc.sh memlock stress-ng 75 >> $filename 2>&1
sleep $sleeptime
./alloc.sh dont-memlock stress-ng 75 >> $filename 2>&1
sleep $sleeptime
./alloc.sh memlock iperf >> $filename 2>&1
sleep $sleeptime
./alloc.sh dont-memlock iperf >> $filename 2>&1
sleep $sleeptime
./alloc.sh memlock hackbench >> $filename 2>&1
sleep $sleeptime
./alloc.sh dont-memlock hackbench >> $filename 2>&1
sleep $sleeptime


echo "Finished alloctest" >> $filename

