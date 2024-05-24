#!/bin/sh
build=$(uname -r)


sleeptime=5s

if [ -n "$1" ]; then
    if [ "$1" == "load" ]; then
        filename=preemption_test_${build}_load.res
    fi
else
    filename=preemption_test_${build}.res
fi

echo "Running preemption test on $build, storing to $filename"  
echo "Running preemption on $build" > $filename

if [ -n "$1" ]; then
    if [ "$1" == "load" ]; then
        iperf3 -s > /dev/null 2>&1 &
        iperf3_server_pid=$!

        iperf3 -c localhost -t 0 > /dev/null 2>&1 &
        iperf3_client_pid=$!
    fi
fi



echo "SCHED_OTHER" >> $filename
./PreemptionTest >> $filename
sleep $sleeptime

echo "SCHED_OTHER, low nice" >> $filename
nice -n -20 ./PreemptionTest >> $filename
sleep $sleeptime

echo "SCHED_FIFO 40" >> $filename
chrt -f 40 ./PreemptionTest >> $filename
sleep $sleeptime

echo "SCHED_FIFO 60" >> $filename
chrt -f 60 ./PreemptionTest >> $filename
sleep $sleeptime

echo "SCHED_FIFO 99" >> $filename
chrt -f 99 ./PreemptionTest >> $filename
sleep $sleeptime

if [ -n "$1" ]; then
    if [ "$1" == "load" ]; then
        echo "Killing iperf" >> $filename
        kill -9 $iperf3_client_pid $iperf3_server_pid
    fi
fi

