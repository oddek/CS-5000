#!/bin/sh

build=$(uname -r)
filename=memory_test_${build}.res

echo "Running memoryTest on $build, storing to $filename"  
echo "" > $filename

echo "Running memoryTest on $build" >> $filename


./memoryTest.sh stack >> $filename
./memoryTest.sh stack prefault >> $filename
./memoryTest.sh static  >> $filename
./memoryTest.sh static prefault >> $filename
./memoryTest.sh heap  >> $filename
./memoryTest.sh heap prefault >> $filename

./memoryTest.sh stack memlock >> $filename
./memoryTest.sh static memlock >> $filename
./memoryTest.sh heap memlock >> $filename

./memoryTest.sh stack memlock-main >> $filename
./memoryTest.sh static memlock-main >> $filename
./memoryTest.sh heap memlock-main >> $filename