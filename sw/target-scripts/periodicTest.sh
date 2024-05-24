#!/bin/sh

dirname=periodicHist
mkdir -p $dirname
build=$(uname -r)


if [ -n "$2" ]; then
load=$2
else
load=25
fi

file="${1}_${load}_${build}"
echo $file
echo "Load $load"

stress-ng -c 2 -l $load &
stress_pid=$!

if [ "$1" == "timer" ]; then
    ./TimerTest timer > $dirname/$file.stat &

elif [ "$1" == "timerFifo" ]; then
    ./TimerTest timer rt > $dirname/$file.stat & 

elif [ "$1" == "timerChrt" ]; then
    chrt 81 ./TimerTest timer > $dirname/$file.stat & 

elif [ "$1" == "nanosleep" ]; then
    ./TimerTest nanosleep > $dirname/$file.stat & 

elif [ "$1" == "nanosleepFifo" ]; then
    ./TimerTest nanosleep rt > $dirname/$file.stat & 

elif [ "$1" == "edf" ]; then
    ./TimerTest edf > $dirname/$file.stat & 

elif [ "$1" == "timedwait" ]; then
    ./TimerTest timedwait > $dirname/$file.stat & 

elif [ "$1" == "timedwaitFifo" ]; then
    ./TimerTest timedwait rt > $dirname/$file.stat & 

else
    echo "Invalid arg"
    kill -9 $stress_pid
    exit 1
fi

test_pid=$!

pidstat -p $test_pid 1 > $dirname/$file.cpu &
pidstat_pid=$!

sleep 1

echo "Waiting for $test_pid"
wait $test_pid

kill -9 $stress_pid $pidstat_pid

cp timerTestHistogram $dirname/$file.hist