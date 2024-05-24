
#!/bin/sh

./unload_verifier
./unload_generator
./load_verifier
./load_generator

build=$(uname -r)
filename=kernel_thread_test_${build}.res

echo "Running kernel thread test on $build, storing to $filename"  

echo "Running kernel thread test on $build, at $(date)" > $filename

RUNTIME="2_m"


./TaskMonitor --runtime $RUNTIME --silent >> $filename 2>&1
./netlinkTest.sh --runtime $RUNTIME --period 100_us >> $filename 2>&1
./netlinkTest.sh --runtime $RUNTIME --period 1_ms >> $filename 2>&1
./netlinkTest.sh --runtime $RUNTIME --period 10_ms >> $filename 2>&1
./netlinkTest.sh --runtime $RUNTIME --period 1000_ms >> $filename 2>&1

./TaskMonitor --runtime $RUNTIME --silent --stress 25 >> $filename 2>&1
./netlinkTest.sh --runtime $RUNTIME --period 100_us --stress 25 >> $filename 2>&1
./netlinkTest.sh --runtime $RUNTIME --period 1_ms --stress 25 >> $filename 2>&1
./netlinkTest.sh --runtime $RUNTIME --period 10_ms --stress 25 >> $filename 2>&1
./netlinkTest.sh --runtime $RUNTIME --period 1000_ms --stress 25 >> $filename 2>&1

echo "Ended kernel thread test on $build, at $(date)" >> $filename
