#!/bin/sh


build=$(uname -r)
filename=load_test_${build}.res

echo "Running loadtest on $build, storing to $filename"  
echo "" > $filename

echo "Running loadtest on $build" >> $filename

./loadTest.sh none >> $filename
./loadTest.sh stress-ng 25 >> $filename
./loadTest.sh iperf >> $filename
./loadTest.sh hackbench >> $filename