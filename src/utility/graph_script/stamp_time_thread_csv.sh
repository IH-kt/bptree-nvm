#!/bin/sh

if [ $# -eq 0 ]; then
    root_dir=..
else
    root_dir=$1
fi
# exec_types='emulator log_comp para_cp use_mmap'
# exec_types='log_comp para_cp use_mmap'
exec_types='log_comp para_cp use_mmap para_cp_dram'
bench_types='genome intruder kmeans-high kmeans-low labyrinth ssca2 vacation-high vacation-low yada'
max_trial=3
trials=`seq 1 ${max_trial}`
result_dir="./scaling"
thrs="1 2 4 8"

function format_switch () {
    source_dir=$1
    bench_type=$2
    thr=$3
    trial=$4
    case $bench_type in
        genome        ) cat ${source_dir}/time_thr${thr}_trial${trial} | grep "Time"    | sed -E 's/.* ([0-9]+\.[0-9]+).*/\1/' ;;
        intruder      ) cat ${source_dir}/time_thr${thr}_trial${trial} | grep "Elapsed" | sed -E 's/.* ([0-9]+\.[0-9]+).*/\1/' ;;
        kmeans-high   ) cat ${source_dir}/time_thr${thr}_trial${trial} | grep "Time"    | sed -E 's/.* ([0-9]+\.[0-9]+).*/\1/' ;;
        kmeans-low    ) cat ${source_dir}/time_thr${thr}_trial${trial} | grep "Time"    | sed -E 's/.* ([0-9]+\.[0-9]+).*/\1/' ;;
        labyrinth     ) cat ${source_dir}/time_thr${thr}_trial${trial} | grep "Elapsed" | sed -E 's/.* ([0-9]+\.[0-9]+).*/\1/' ;;
        vacation-high ) cat ${source_dir}/time_thr${thr}_trial${trial} | grep "Time"    | sed -E 's/.* ([0-9]+\.[0-9]+).*/\1/' ;;
        vacation-low  ) cat ${source_dir}/time_thr${thr}_trial${trial} | grep "Time"    | sed -E 's/.* ([0-9]+\.[0-9]+).*/\1/' ;;
        ssca2         ) cat ${source_dir}/time_thr${thr}_trial${trial} | grep "Time"    | sed -E 's/.* ([0-9]+\.[0-9]+).*/\1/' ;;
        yada          ) cat ${source_dir}/time_thr${thr}_trial${trial} | grep "Elapsed" | sed -E 's/.* ([0-9]+\.[0-9]+).*/\1/' ;;
    esac
}

function make_csv () {
    bench_type=$1
    source_dir=$2
    target_dir=$3

    echo 'thread,time' > ${target_dir}/time_${bench_type}.csv
    source_dir_tmp=`echo "$source_dir/$bench_type"`
    for thr in $thrs
    do
        time_sum=0
        for trial in $trials
        do
            elapsed_time=`format_switch $source_dir_tmp $bench_type $thr $trial`
            time_sum=`echo "scale=7; ${elapsed_time} + ${time_sum}" | bc`
        done
        time_pt=`echo "scale=7; (${time_sum} / ${max_trial})" | bc`
        echo "$thr,$time_pt" >> ${target_dir}/time_${bench_type}.csv
    done
}

function apply_all_bench () {
    exec_type=$1
    root_dir=$2
    result_dir=$3
    mkdir -p $result_dir/$exec_type
    for bench_type in $bench_types
    do
        make_csv $bench_type $root_dir/test_$exec_type $result_dir/$exec_type
    done
}

mkdir -p ${result_dir}
for exec_type in $exec_types
do
    apply_all_bench $exec_type $root_dir $result_dir
done
