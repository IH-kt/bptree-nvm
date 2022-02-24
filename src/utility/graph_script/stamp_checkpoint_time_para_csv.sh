#!/bin/sh

if [ $# -eq 0 ]; then
    root_dir=..
else
    root_dir=$1
fi

# exec_types='log_comp para_cp use_mmap para_cp_dram'
# exec_types='para_cp log_comp_single log_comp log_comp_lp log_comp_lp_single'
exec_types='log_comp_lp log_comp_lp_single'
bench_types='genome intruder kmeans-high kmeans-low labyrinth ssca2 vacation-high vacation-low yada'
max_trial=3
trials=`seq 1 ${max_trial}`
result_dir="./checkpointtime"
thrs="1 2 4 8"

function make_csv () {
    bench_type=$1
    source_dir=$2
    target_dir=$3

    echo making ${target_dir}/checkpointtime_${bench_type}.csv
    echo 'thread,reserve,commit-finding,apply-log-flush' > ${target_dir}/checkpointtime_${bench_type}.csv
    source_dir_tmp=`echo "$source_dir/$bench_type"`

    for thr in $thrs
    do
        rcheck_time_sum=0
        ccheck_time_sum=0
        acheck_time_sum=0
        for trial in $trials
        do
            rcheck_time=`cat ${source_dir_tmp}/result_thr${thr}_trial${trial} | grep "sec. 1" | cut -f 6 -d ' '`
            ccheck_time=`cat ${source_dir_tmp}/result_thr${thr}_trial${trial} | grep "sec. 2" | cut -f 7 -d ' '`
            acheck_time=`cat ${source_dir_tmp}/result_thr${thr}_trial${trial} | grep "sec. 3" | cut -f 8 -d ' '`
            echo $acheck_time
            rcheck_time_sum=`echo "scale=7; ${rcheck_time} + ${rcheck_time_sum}" | bc`
            ccheck_time_sum=`echo "scale=7; ${ccheck_time} + ${ccheck_time_sum}" | bc`
            acheck_time_sum=`echo "scale=7; ${acheck_time} + ${acheck_time_sum}" | bc`
        done
        rcheck_time_avg=`echo "scale=7; (${rcheck_time_sum} / ${max_trial})" | bc`
        ccheck_time_avg=`echo "scale=7; (${ccheck_time_sum} / ${max_trial})" | bc`
        acheck_time_avg=`echo "scale=7; (${acheck_time_sum} / ${max_trial})" | bc`
        echo "$thr,$rcheck_time_avg,$ccheck_time_avg,$acheck_time_avg" >> ${target_dir}/checkpointtime_${bench_type}.csv
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
