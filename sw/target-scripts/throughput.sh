#!/bin/sh

uname -a

# time hackbench -l 50000 --pipe
# time chrt -f 10 hackbench -l 5000 --pipe
# time hackbench -l 50000
# time chrt -f 10 hackbench -l 5000


iperf3 -s > /dev/null 2>&1 &
server_pid=$!
time iperf3 -c localhost -i 0 -k 500000 --omit 10

kill -9 $server_pid
