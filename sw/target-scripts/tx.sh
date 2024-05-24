#!/bin/sh

if [ $# -eq 2 ]; then
sleeptime=${2}
else
sleeptime=0.01
fi

echo "Transmitting ${1}B packages at ${sleeptime} s intervals"

i=1
while [ "$i" -ne 0 ]
do
  echo "${1}" > /dev/generator
  sleep $sleeptime
done