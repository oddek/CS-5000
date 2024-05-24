#!/bin/sh

set -e

runtime=$((60*30))
# runtime=5


echo "Starting ${1} test at $(date), will run for ${runtime} seconds"

if [ "$1" == "iperf" ]; then

    iperf3 -s > /dev/null 2>&1 &
    iperf3_server_pid=$!

    iperf3 -c localhost -t 0 -u --bidir -b 500M > /dev/null 2>&1 &
    iperf3_client_pid=$!

    killList="${iperf3_client_pid} ${iperf3_server_pid}"

elif [ "$1" == "stress-ng" ]; then

    echo "stress-ng load $2"

    stress-ng -c 2 -l $2 &
    killList=$!

elif [ "$1" == "hackbench" ]; then

    hackbench --loops=1000000000 -g 1 -f 1 -s 1 > /dev/null 2>&1 & 
    killList=$!

elif [ "$1" == "none" ]; then
    x=""
fi

sleep ${runtime}

cat /proc/loadavg

if [ -n "$killList" ]; then
    kill -9 $killList
fi


echo "Finished ${1} test at $(date)"
printf "\n"
