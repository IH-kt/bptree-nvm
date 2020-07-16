#!/bin/sh

if [ $# -eq 0 ]; then
    root_dir=..
else
    root_dir=$1
fi

exec_types='log_comp para_cp use_mmap para_cp_dram'
bench_types='genome intruder kmeans-high kmeans-low labyrinth ssca2 vacation-high vacation-low yada'
max_trial=3
trials=`seq 1 ${max_trial}`
result_dir="./aborts"
thrs="1 2 4 8"

function make_csv () {
    bench_type=$1
    source_dir=$2
    target_dir=$3

    echo 'thread,abort,conflict,capacity,explicit' > ${target_dir}/abort_${bench_type}.csv
    source_dir_tmp=`echo "$source_dir/$bench_type"`
    for thr in $thrs
    do
        abort_tmp=0
        conflict_tmp=0
        capacity_tmp=0
        explicit_tmp=0
        for trial in $trials
        do
            tmp=`grep "ABORTS :" ${source_dir_tmp}/result_thr${thr}_trial${trial} | cut -f 3 -d ' '`
            abort_tmp=`echo "scale=7; ${abort_tmp} + ${tmp}" | bc`
            tmp=`grep "CONFLS :" ${source_dir_tmp}/result_thr${thr}_trial${trial} | cut -f 3 -d ' '`
            conflict_tmp=`echo "scale=7; ${conflict_tmp} + ${tmp}" | bc`
            tmp=`grep "CAPACS :" ${source_dir_tmp}/result_thr${thr}_trial${trial} | cut -f 3 -d ' '`
            capacity_tmp=`echo "scale=7; ${capacity_tmp} + ${tmp}" | bc`
            tmp=`grep "EXPLIC :" ${source_dir_tmp}/result_thr${thr}_trial${trial} | cut -f 3 -d ' '`
            explicit_tmp=`echo "scale=7; ${explicit_tmp} + ${tmp}" | bc`
        done
        abort=`echo "scale=7; ${abort_tmp} / ${max_trial}" | bc`
        conflict=`echo "scale=7; ${conflict_tmp} / ${max_trial}" | bc`
        capacity=`echo "scale=7; ${capacity_tmp} / ${max_trial}" | bc`
        explicit=`echo "scale=7; ${explicit_tmp} / ${max_trial}" | bc`
        echo "$thr,$abort,$conflict,$capacity,$explicit" >> ${target_dir}/abort_${bench_type}.csv
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
