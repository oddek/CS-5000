#!/bin/sh
build=$(uname -r)

filename=cyclic_${build}.res
sleeptime=5

echo "Running cyclictest on $build, storing to $filename"  
echo "Running cyclictest on $build" > $filename
echo " " >> $filename

./cyclic.sh other >> $filename 2>&1
sleep $sleeptime
./cyclic.sh othernice >> $filename 2>&1
sleep $sleeptime
./cyclic.sh fifo40 >> $filename 2>&1
sleep $sleeptime
./cyclic.sh fifo60 >> $filename 2>&1
sleep $sleeptime
./cyclic.sh fifo99 >> $filename 2>&1
sleep $sleeptime

./cyclic.sh other stress-ng 25 >> $filename 2>&1
sleep $sleeptime
./cyclic.sh othernice stress-ng 25 >> $filename 2>&1
sleep $sleeptime
./cyclic.sh fifo40 stress-ng 25 >> $filename 2>&1
sleep $sleeptime
./cyclic.sh fifo60 stress-ng 25 >> $filename 2>&1
sleep $sleeptime
./cyclic.sh fifo99 stress-ng 25 >> $filename 2>&1
sleep $sleeptime

./cyclic.sh other stress-ng 75 >> $filename 2>&1
sleep $sleeptime
./cyclic.sh othernice stress-ng 75 >> $filename 2>&1
sleep $sleeptime
./cyclic.sh fifo40 stress-ng 75 >> $filename 2>&1
sleep $sleeptime
./cyclic.sh fifo60 stress-ng 75 >> $filename 2>&1
sleep $sleeptime
./cyclic.sh fifo99 stress-ng 75 >> $filename 2>&1
sleep $sleeptime


./cyclic.sh other iperf >> $filename 2>&1
sleep $sleeptime
./cyclic.sh othernice iperf >> $filename 2>&1
sleep $sleeptime
./cyclic.sh fifo40 iperf >> $filename 2>&1
sleep $sleeptime
./cyclic.sh fifo60 iperf >> $filename
sleep $sleeptime 
./cyclic.sh fifo99 iperf >> $filename 2>&1
sleep $sleeptime