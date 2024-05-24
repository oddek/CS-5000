#!/bin/sh


build=$(uname -r)
filename=full_test_rx_${build}.res

echo "Running FULL test RX on $build, storing to $filename"  

echo "Running FULL test RX on $build, at $(date)" > $filename

./loaddrivers.sh

echo "********" >> $filename
echo "RX TESTS" >> $filename
echo "********" >> $filename

./fullTestRx.sh >> $filename 2>&1
./fullTestRx.sh --runmore >> $filename 2>&1
./fullTestRx.sh --realtime >> $filename 2>&1
./fullTestRx.sh --realtime --runmore >> $filename 2>&1

./fullTestRx.sh --stress 25 >> $filename 2>&1
./fullTestRx.sh --stress 25 --runmore >> $filename 2>&1
./fullTestRx.sh --stress 25 --realtime >> $filename 2>&1
./fullTestRx.sh --stress 25 --realtime --runmore >> $filename 2>&1

./fullTestRx.sh --stress 75 >> $filename 2>&1
./fullTestRx.sh --stress 75 --runmore >> $filename 2>&1
./fullTestRx.sh --stress 75 --realtime >> $filename 2>&1
./fullTestRx.sh --stress 75 --realtime --runmore >> $filename 2>&1

./fullTestRx.sh --iperf >> $filename 2>&1
./fullTestRx.sh --iperf --runmore >> $filename 2>&1
./fullTestRx.sh --iperf --realtime >> $filename 2>&1
./fullTestRx.sh --iperf --realtime --runmore >> $filename 2>&1


echo "Ended full test on $build, at $(date)" >> $filename
