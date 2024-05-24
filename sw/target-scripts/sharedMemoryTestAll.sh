#!/bin/sh


build=$(uname -r)
filename=shared_memory_test_${build}.res

echo "Running shared memory test on $build, storing to $filename"  


echo "Running shared memory test on $build, at $(date)" > $filename

./load_cma

./sharedMemoryTest.sh >> $filename 2>&1
./sharedMemoryTest.sh --runmore >> $filename 2>&1
./sharedMemoryTest.sh --realtime >> $filename 2>&1
./sharedMemoryTest.sh --realtime --runmore >> $filename 2>&1

./sharedMemoryTest.sh --stress 25 >> $filename 2>&1
./sharedMemoryTest.sh --runmore --stress 25 >> $filename 2>&1
./sharedMemoryTest.sh --realtime --stress 25 >> $filename 2>&1
./sharedMemoryTest.sh --realtime --runmore --stress 25 >> $filename 2>&1

./sharedMemoryTest.sh --stress 75 >> $filename 2>&1
./sharedMemoryTest.sh --runmore --stress 75 >> $filename 2>&1
./sharedMemoryTest.sh --realtime --stress 75 >> $filename 2>&1
./sharedMemoryTest.sh --realtime --runmore --stress 75 >> $filename 2>&1

./sharedMemoryTest.sh --iperf >> $filename 2>&1
./sharedMemoryTest.sh --runmore --iperf >> $filename 2>&1
./sharedMemoryTest.sh --realtime --iperf >> $filename 2>&1
./sharedMemoryTest.sh --realtime --runmore --iperf >> $filename 2>&1


echo "Ended shared memory test on $build, at $(date)" >> $filename
