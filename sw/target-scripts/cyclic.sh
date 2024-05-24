#!/bin/sh

set -e

if [ "$1" == "other" ]; then
    policy=normal
    prefix=""
    prio=""

elif [ "$1" == "othernice" ]; then
    policy=normal
    prefix="nice -n -20 "
    prio=""

elif [ "$1" == "fifo40" ]; then
    policy=fifo
    prefix=""
    prio="40"

elif [ "$1" == "fifo60" ]; then
    policy=fifo
    prefix=""
    prio="60"

elif [ "$1" == "fifo99" ]; then
    policy=fifo
    prefix=""
    prio="99"
fi


if [ -n "$prio" ]; then
    priorityarg="-p ${prio}"
else
    priorityarg=""
fi

if [ -n "$2" ]; then
    if [ "$2" == "stress-ng" ]; then
        load="$2 $3"
        stress-ng -c 2 -l "$3" &
        stress_pid=$!
        killList=$stress_pid
        sleep 2
    elif [ "$2" == "iperf" ]; then
        iperf3 -s > /dev/null 2>&1 &
        iperf3_server_pid=$!

        iperf3 -c localhost -t 0 > /dev/null 2>&1 &
        iperf3_client_pid=$!
        load="iperf"
        killList="$iperf3_client_pid $iperf3_server_pid"
        sleep 2
    elif [ "$2" == "hackbench" ]; then
        hackbench --loops 10000000000 -g 1 -f 1 -s 1 &
        hackbench_pid=$!
        load="hackbench"
        killList="$hackbench_pid"
        sleep 2
    else
        load=0
    fi
else
    load=0
fi

echo "Running ${1} test, with load at ${load} $(date)"

${prefix}cyclictest -l 500000 --threads 1 -i 400 -m --smp $priorityarg --policy ${policy} --default-system --quiet

if [ -n "$killList" ]; then
    kill -9 $killList
fi

echo "${1} test ended at $(date)"
echo " "
echo " "