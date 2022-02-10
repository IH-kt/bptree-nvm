#!/bin/sh

if [ $# -eq 0 ]; then
    root_dir=./result_kmeans-high_test_REDO-TS-FORK
else
    root_dir=$1
fi
plain=bptree_nvhtm_0
types='plain'
# ops='insert delete search'
bench_var='kmeans'
# max_trial=5
max_trial=3
trials=`seq 1 ${max_trial}`
result_dir="$root_dir/thread_csv"
thrs="1 4 8"

mkdir -p ${result_dir}
for b in $bench_var
do
    echo 'thread,time' > ${result_dir}/time_${b}.csv
    target_dir=`echo "${root_dir}/"`
    for thr in $thrs
    do
        time_sum=0
        for trial in $trials
        do
            elapsed_time=`cat ${target_dir}/time_thr${thr}_trial${trial} | grep "Time" | cut -f 2 -d ' '`
            time_sum=`echo "scale=7; ${elapsed_time} + ${time_sum}" | bc`
        done
        time_pt=`echo "scale=7; (${time_sum} / ${max_trial})" | bc`
        echo "$thr,$time_pt" >> ${result_dir}/time_${b}.csv
    done
done
