#!/bin/sh


build=$(uname -r)
filename=generic_netlink_test_${build}.res

echo "Running generic netlink test on $build, storing to $filename"  


echo "Running generic netlink test on $build, at $(date)" > $filename


./netlinkTest.sh >> $filename 2>&1
./netlinkTest.sh --realtime >> $filename 2>&1

./netlinkTest.sh --stress 25 >> $filename 2>&1
./netlinkTest.sh --realtime --stress 25 >> $filename 2>&1

./netlinkTest.sh --stress 75 >> $filename 2>&1
./netlinkTest.sh --realtime --stress 75 >> $filename 2>&1

./netlinkTest.sh --iperf >> $filename 2>&1
./netlinkTest.sh --realtime --iperf >> $filename 2>&1

./netlinkTest.sh --realtime-generator --realtime >> $filename 2>&1
./netlinkTest.sh --realtime-generator --realtime --stress 25 >> $filename 2>&1
./netlinkTest.sh --realtime-generator --realtime --stress 75 >> $filename 2>&1
./netlinkTest.sh --realtime-generator --realtime --iperf >> $filename 2>&1


echo "Ended generic netlink test on $build, at $(date)" >> $filename
