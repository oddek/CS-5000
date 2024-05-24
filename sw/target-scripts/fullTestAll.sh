#!/bin/sh


build=$(uname -r)
filename=full_test_${build}.res

echo "Running FULL test on $build, storing to $filename"  

echo "Running FULL test on $build, at $(date)" > $filename

./loaddrivers.sh

echo "********" >> $filename
echo "TX TESTS" >> $filename
echo "********" >> $filename

./fullTestTx.sh >> $filename 2>&1
./fullTestTx.sh --realtime >> $filename 2>&1
./fullTestTx.sh --runmore >> $filename 2>&1
./fullTestTx.sh --realtime --runmore >> $filename 2>&1
./fullTestTx.sh --realtime-generator >> $filename 2>&1
./fullTestTx.sh --realtime-generator --realtime >> $filename 2>&1
./fullTestTx.sh --realtime-generator --runmore >> $filename 2>&1
./fullTestTx.sh --realtime-generator --realtime --runmore >> $filename 2>&1

./fullTestTx.sh --stress 25 >> $filename 2>&1
./fullTestTx.sh --stress 25 --realtime >> $filename 2>&1
./fullTestTx.sh --stress 25 --runmore >> $filename 2>&1
./fullTestTx.sh --stress 25 --realtime --runmore >> $filename 2>&1
./fullTestTx.sh --stress 25 --realtime-generator >> $filename 2>&1
./fullTestTx.sh --stress 25 --realtime-generator --realtime >> $filename 2>&1
./fullTestTx.sh --stress 25 --realtime-generator --runmore >> $filename 2>&1
./fullTestTx.sh --stress 25 --realtime-generator --realtime --runmore >> $filename 2>&1

./fullTestTx.sh --stress 75 >> $filename 2>&1
./fullTestTx.sh --stress 75 --realtime >> $filename 2>&1
./fullTestTx.sh --stress 75 --runmore >> $filename 2>&1
./fullTestTx.sh --stress 75 --realtime --runmore >> $filename 2>&1
./fullTestTx.sh --stress 75 --realtime-generator >> $filename 2>&1
./fullTestTx.sh --stress 75 --realtime-generator --realtime >> $filename 2>&1
./fullTestTx.sh --stress 75 --realtime-generator --runmore >> $filename 2>&1
./fullTestTx.sh --stress 75 --realtime-generator --realtime --runmore >> $filename 2>&1

./fullTestTx.sh --iperf >> $filename 2>&1
./fullTestTx.sh --iperf --realtime >> $filename 2>&1
./fullTestTx.sh --iperf --runmore >> $filename 2>&1
./fullTestTx.sh --iperf --realtime --runmore >> $filename 2>&1
./fullTestTx.sh --iperf --realtime-generator >> $filename 2>&1
./fullTestTx.sh --iperf --realtime-generator --realtime >> $filename 2>&1
./fullTestTx.sh --iperf --realtime-generator --runmore >> $filename 2>&1
./fullTestTx.sh --iperf --realtime-generator --realtime --runmore >> $filename 2>&1

echo "********" >> $filename
echo "RX TESTS" >> $filename
echo "********" >> $filename

./fullTestRx.sh >> $filename 2>&1
./fullTestRx.sh --realtime >> $filename 2>&1
./fullTestRx.sh --runmore >> $filename 2>&1
./fullTestRx.sh --realtime --runmore >> $filename 2>&1

./fullTestRx.sh --stress 25 >> $filename 2>&1
./fullTestRx.sh --stress 25 --realtime >> $filename 2>&1
./fullTestRx.sh --stress 25 --runmore >> $filename 2>&1
./fullTestRx.sh --stress 25 --realtime --runmore >> $filename 2>&1

./fullTestRx.sh --stress 75 >> $filename 2>&1
./fullTestRx.sh --stress 75 --realtime >> $filename 2>&1
./fullTestRx.sh --stress 75 --runmore >> $filename 2>&1
./fullTestRx.sh --stress 75 --realtime --runmore >> $filename 2>&1

./fullTestRx.sh --iperf >> $filename 2>&1
./fullTestRx.sh --iperf --realtime >> $filename 2>&1
./fullTestRx.sh --iperf --runmore >> $filename 2>&1
./fullTestRx.sh --iperf --realtime --runmore >> $filename 2>&1



echo "Ended generic netlink test on $build, at $(date)" >> $filename
