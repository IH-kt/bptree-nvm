#!/bin/sh

if [ $# -eq 0 ]; then
    root_dir=..
else
    root_dir=$1
fi
# exec_types='emulator log_comp para_cp use_mmap'
# exec_types='log_comp para_cp use_mmap'
# exec_types='log_comp para_cp use_mmap para_cp_dram'
exec_types='log_comp use_mmap use_mmap_dram no_cp no_cp_dram'
bench_types='genome intruder kmeans-high kmeans-low labyrinth ssca2 vacation-high vacation-low yada'
max_trial=3
trials=`seq 1 ${max_trial}`
result_dir="./committime"
thrs="1 2 4 8"

function make_csv () {
    bench_type=$1
    source_dir=$2
    target_dir=$3

    echo 'thread,clwb,otherwait,fence' > ${target_dir}/commit_${bench_type}.csv
    source_dir_tmp=`echo "$source_dir/$bench_type"`
    for thr in $thrs
    do
        clwb_sum=0
        otherwait_sum=0
        fence_sum=0
        for trial in $trials
        do
            clwb=`cat ${source_dir_tmp}/result_thr${thr}_trial${trial} | grep "COMMIT_TIME1:" | cut -f 2 -d ' '`
            otherwait=`cat ${source_dir_tmp}/result_thr${thr}_trial${trial} | grep "COMMIT_TIME2:" | cut -f 2 -d ' '`
            fence=`cat ${source_dir_tmp}/result_thr${thr}_trial${trial} | grep "COMMIT_TIME3:" | cut -f 2 -d ' '`
            clwb_sum=`echo "scale=7; ${clwb} + ${clwb_sum}" | bc`
            otherwait_sum=`echo "scale=7; ${otherwait} + ${otherwait_sum}" | bc`
            fence_sum=`echo "scale=7; ${fence} + ${fence_sum}" | bc`
        done

        clwb_thr=`echo "scale=7; (${clwb_sum} / ${max_trial}) / ${thr}" | bc`
        otherwait_thr=`echo "scale=7; (${otherwait_sum} / ${max_trial}) / ${thr}" | bc`
        fence_thr=`echo "scale=7; (${fence_sum} / ${max_trial}) / ${thr}" | bc`
        echo "$thr,$clwb_thr,$otherwait_thr,$fence_thr" >> ${target_dir}/commit_${bench_type}.csv
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
