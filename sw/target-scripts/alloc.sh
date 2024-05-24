
#!/bin/sh

set -e

if [ "$1" == "memlock" ]; then
    memlock=""
elif [ "$1" == "dont-memlock" ]; then
    memlock="--dont-memlock-on-rt"
else
    echo "Usage: $0 [memlock|dont-memlock] [stress-ng|iperf|hackbench] [stress-ng load]"
    exit 1
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

        iperf3 -c localhost -t 3600 > /dev/null 2>&1 &
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

echo "Running Alloctest, with load at ${load} $(date)"

./AllocTest --runtime "20_m" --realtime --silent $memlock

if [ -n "$killList" ]; then
    echo "Killing $killList"
    kill -9 $killList
fi

if [ "$load" == "hackbench" ]; then
    echo "Killing hackbench"
    killall hackbench
fi

echo "${1} test ended at $(date)"
echo " "
echo " "
