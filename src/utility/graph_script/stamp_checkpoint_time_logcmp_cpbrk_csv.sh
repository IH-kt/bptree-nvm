#!/bin/sh

if [ $# -eq 0 ]; then
    root_dir=..
else
    root_dir=$1
fi

# exec_types='log_comp para_cp use_mmap para_cp_dram'
exec_types='log_comp_lp_cpbrk log_comp_lp_single_cpbrk'
bench_types='genome intruder kmeans-high kmeans-low labyrinth ssca2 vacation-high vacation-low yada'
max_trial=3
trials=`seq 1 ${max_trial}`
result_dir="./checkpointtime"
thrs="1 2 4 8"

function make_csv () {
    bench_type=$1
    source_dir=$2
    target_dir=$3

    echo 'thread,reserve,commit-finding,log-read,library,logwrite,apply-flush' >  ${target_dir}/checkpointtime_${bench_type}.csv
    source_dir_tmp=`echo "$source_dir/$bench_type"`

    for thr in $thrs
    do
        reserve_time_sum=0
        commit_find_time_sum=0
        logwrite_time_sum=0
        read_only_time_sum=0
        no_write_time_sum=0
        write_flush_time_sum=0
        for trial in $trials
        do
            reserve_time=`cat ${source_dir_tmp}/result_thr${thr}_trial${trial} | grep "sec. 1" | cut -f 6 -d ' '`
            commit_find_time=`cat ${source_dir_tmp}/result_thr${thr}_trial${trial} | grep "sec. 2" | cut -f 7 -d ' '`
            logwrite_time=`cat ${source_dir_tmp}/result_thr${thr}_trial${trial} | grep "d lo" | cut -f 5 -d ' '`
            read_only_time=`cat ${source_dir_tmp}/result_thr${thr}_trial${trial} | grep "d re" | cut -f 5 -d ' '`
            no_write_time=`cat ${source_dir_tmp}/result_thr${thr}_trial${trial} | grep "d no" | cut -f 5 -d ' '`
            write_flush_time=`cat ${source_dir_tmp}/result_thr${thr}_trial${trial} | grep "d wr" | cut -f 5 -d ' '`
            reserve_time_sum=`echo "scale=7; ${reserve_time} + ${reserve_time_sum}" | bc`
            commit_find_time_sum=`echo "scale=7; ${commit_find_time} + ${commit_find_time_sum}" | bc`
            logwrite_time_sum=`echo "scale=7; ${logwrite_time} + ${logwrite_time_sum}" | bc`
            read_only_time_sum=`echo "scale=7; ${read_only_time} + ${read_only_time_sum}" | bc`
            no_write_time_sum=`echo "scale=7; ${no_write_time} + ${no_write_time_sum}" | bc`
            write_flush_time_sum=`echo "scale=7; ${write_flush_time} + ${write_flush_time_sum}" | bc`
        done
        rsrv_time_avg=`echo "scale=7; (${reserve_time_sum} / ${max_trial})" | bc`
        cmfd_time_avg=`echo "scale=7; (${commit_find_time_sum} / ${max_trial})" | bc`
        lwrt_time_avg=`echo "scale=7; (${logwrite_time_sum} / ${max_trial})" | bc`
        read_time_avg=`echo "scale=7; (${read_only_time_sum} / ${max_trial})" | bc`
        nowr_time_avg=`echo "scale=7; (${no_write_time_sum} / ${max_trial})" | bc`
        wrfl_time_avg=`echo "scale=7; (${write_flush_time_sum} / ${max_trial})" | bc`
        write_time_diff=`echo "scale=7; ${lwrt_time_avg} - ${nowr_time_avg}" | bc`
        libry_time_diff=`echo "scale=7; ${nowr_time_avg} - ${read_time_avg}" | bc`
        echo "$thr,$rsrv_time_avg,$cmfd_time_avg,$read_time_avg,$libry_time_diff,$write_time_diff,$wrfl_time_avg" >> ${target_dir}/checkpointtime_${bench_type}.csv
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
