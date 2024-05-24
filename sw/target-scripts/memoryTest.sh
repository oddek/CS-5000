#!/bin/sh

max_samples=5000

soft_faults=0
soft_faults_min=0
soft_faults_max=0
hard_faults=0
hard_faults_min=0
hard_faults_max=0
runtime=0
runtime_min=9999999
runtime_max=0
highest_single_fault_count=0
highest_single_fault_latency=0
average_single_fault_latency=0
samples=0

do_test()
{
    output=$(./MemoryTest $1 $2)
    output=$(echo "$output" | tail -n 6)

    curr_soft_faults=$(echo "$output" | awk '/Soft Page Faults:/ {print $4}')
    curr_hard_faults=$(echo "$output" | awk '/Hard Page Faults:/ {print $4}')
    curr_runtime=$(echo "$output" | awk '/Took/ {print $2}')
    curr_highest_single_fault_count=$(echo "$output" | awk '/Highest Single Fault Count:/ {print $5}')
    curr_highest_single_fault_latency=$(echo "$output" | awk '/Highest Single Fault Latency:/ {print $5}')
    curr_average_single_fault_latency=$(echo "$output" | awk '/Average Single Fault Latency:/ {print $5}')

    if [ "$curr_runtime" -gt "$runtime_max" ]; then
        runtime_max=$curr_runtime
    fi

    if [ "$runtime_min" -gt "$curr_runtime" ]; then
        runtime_min=$curr_runtime
    fi

    if [ "$curr_soft_faults" -gt "$soft_faults_max" ]; then
        soft_faults_max=$curr_soft_faults
    fi

    if [ "$soft_faults_min" -gt "$curr_soft_faults" ]; then
        soft_faults_min=$curr_soft_faults
    fi

    if [ "$curr_hard_faults" -gt "$hard_faults_max" ]; then
        hard_faults_max=$curr_hard_faults
    fi

    if [ "$hard_faults_min" -gt "$curr_hard_faults" ]; then
        hard_faults_min=$curr_hard_faults
    fi

    if [ "$curr_highest_single_fault_latency" -gt "$highest_single_fault_latency" ]; then
        highest_single_fault_latency=$curr_highest_single_fault_latency
    fi

    if [ "$curr_highest_single_fault_count" -gt "$highest_single_fault_count" ]; then
        highest_single_fault_count=$curr_highest_single_fault_count
    fi

    average_single_fault_latency=$(echo "$average_single_fault_latency + $curr_average_single_fault_latency" | bc)

    soft_faults=$(( $soft_faults + $curr_soft_faults))
    hard_faults=$(( $hard_faults + $curr_hard_faults))
    runtime=$(( $runtime + $curr_runtime))
    samples=$(( $samples + 1))
}

echo "Memory Test $1 $2, starting at $(date), samples: $max_samples" 

for i in `seq 2 $max_samples`
do
    do_test $1 $2
done

avg_soft=$(echo $soft_faults / $samples | bc -l)
avg_hard=$(echo $hard_faults / $samples | bc -l)
avg_runtime=$(echo $runtime / $samples | bc -l)

average_single_fault_latency=$(echo "scale=3;  $average_single_fault_latency / $samples" | bc -l)

echo "Soft Faults Avg: $avg_soft"
echo "Soft Faults Min: $soft_faults_min"
echo "Soft Faults Max: $soft_faults_max"
echo "Hard Faults Avg: $avg_hard"
echo "Hard Faults Min: $hard_faults_min"
echo "Hard Faults Max: $hard_faults_max"
echo "Runtime Avg: $avg_runtime us"
echo "Runtime Min: $runtime_min us"
echo "Runtime Max: $runtime_max us"
echo "Highest Single Fault Count: $highest_single_fault_count"
echo "Highest Single Fault Latency: $highest_single_fault_latency ns"
echo "Average Single Fault Latency: $average_single_fault_latency ns"

echo "Memory Test $1 $2, finished at $(date)" 
printf "\n\n"
