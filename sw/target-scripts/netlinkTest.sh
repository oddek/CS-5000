#!/bin/sh

echo "Starting test at $(date)"  

./unload_verifier
./unload_generator
./load_verifier
./load_generator

PERIOD="15_ms"
RUNTIME="10_m"

while [[ $# -gt 0 ]]; do
  case $1 in
    -t|--runtime)
      RUNTIME="$2"
      shift 
      shift 
      ;;
    -p|--period)
      PERIOD="$2"
      shift 
      shift 
      ;;
    -s|--stress)
      STRESS="$2"
      shift 
      shift 
      ;;
    --iperf)
      STRESS="iperf"
      shift 
      ;;
    --realtime)
      REALTIME="--realtime"
      echo "Realtime enabled"
      shift 
      ;;
    --realtime-generator)
      REALTIME="--realtime"
      gen_pid=$(ps aux -o pid,comm | grep GeneratorThread | awk '{print $1}')
      chrt -f -p 55 $gen_pid

      echo "Realtime generator enabled"
      shift 
      ;;
    -*|--*)
      echo "Unknown option $1"
      exit 1
      ;;
    *)

      ;;
  esac
done

echo $STRESS

if [ -n "$STRESS" ]; then
    if [[ "$STRESS" =~ ^[0-9]+$ ]]; then
        load="stress-ng $STRESS"
        stress-ng -c 2 -l "$STRESS" &
        stress_pid=$!
        killList=$stress_pid
        sleep 2
    elif [ "$STRESS" == "iperf" ]; then
        iperf3 -s > /dev/null 2>&1 &
        iperf3_server_pid=$!

        iperf3 -c localhost -t 0 -u --bidir -b 500M > /dev/null 2>&1 &
        iperf3_client_pid=$!
        load="iperf"
        killList="$iperf3_client_pid $iperf3_server_pid"
        sleep 2
    fi
else
    load=0
fi

echo $load

./GenericNetlinkBenchmark --runtime $RUNTIME \
                          --period $PERIOD $REALTIME \
                          --silent

if [ -n "$killList" ]; then
    kill -9 $killList
fi

echo "Ended test $(date)"
echo " "
echo " "