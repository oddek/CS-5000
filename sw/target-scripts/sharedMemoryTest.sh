#!/bin/sh

echo "Starting test at $(date)"  

while [[ $# -gt 0 ]]; do
  case $1 in
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
    --runmore)
      RUNMORE="--aperiodic-sleep --use-run-more"
      echo "RunMore enabled"
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


./SharedMemoryBenchmark --runtime "20_m" \
                        --bitrate 20_Mibps \
                        --period 1_ms $RUNMORE $REALTIME \
                        --silent \
                        --histfile "sharedmem/$(uname -r)_${STRESS}${REALTIME}${RUNMORE}.hist" \
                        --histsize 20_ms


if [ -n "$killList" ]; then
    kill -9 $killList
fi

echo "Ended test $(date)"
echo " "
echo " "