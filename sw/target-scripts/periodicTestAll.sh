#!/bin/sh

if [ -n "$1" ]; then
load=$1
fi

./periodicTest.sh timer $load
./periodicTest.sh timerFifo $load
./periodicTest.sh timerChrt $load
./periodicTest.sh nanosleep $load
./periodicTest.sh nanosleepFifo $load
./periodicTest.sh edf $load
./periodicTest.sh timedwait $load
./periodicTest.sh timedwaitFifo $load