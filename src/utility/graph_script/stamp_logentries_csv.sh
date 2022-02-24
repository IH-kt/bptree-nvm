#!/bin/sh

if [ $# -eq 0 ]; then
    root_dir=..
else
    root_dir=$1
fi
# exec_types='emulator log_comp para_cp use_mmap'
# exec_types='log_comp para_cp use_mmap'
# exec_types='log_comp para_cp use_mmap para_cp_dram'
# exec_types='log_comp use_mmap use_mmap_dram no_cp no_cp_dram'
exec_types='log_comp_lp_noe'
bench_types='genome intruder kmeans-high kmeans-low labyrinth ssca2 vacation-high vacation-low yada'
max_trial=3
trials=`seq 1 ${max_trial}`
result_dir="./entries"
thrs="1 2 4 8"

function make_csv () {
    bench_type=$1
    source_dir=$2
    target_dir=$3

    echo 'thread,all,skipped,ratio' > ${target_dir}/entries_${bench_type}.csv
    source_dir_tmp=`echo "$source_dir/$bench_type"`
    for thr in $thrs
    do
        all_sum=0
        skipped_sum=0
        ratio_sum=0
        for trial in $trials
        do
            all_tmp=`cat ${source_dir_tmp}/result_thr${thr}_trial${trial} | grep "all" | cut -f 5 -d ' '`
            skipped_tmp=`cat ${source_dir_tmp}/result_thr${thr}_trial${trial} | grep "compressed" | cut -f 5 -d ' '`
            all_sum=`echo "scale=7; ${all_tmp} + ${all_sum}" | bc`
            skipped_sum=`echo "scale=7; ${skipped_tmp} + ${skipped_sum}" | bc`
            if [ $all_tmp != "0" ]; then
                ratio_tmp=`cat ${source_dir_tmp}/result_thr${thr}_trial${trial} | grep "compression" | cut -f 5 -d ' '`
                ratio_sum=`echo "scale=7; ${ratio_tmp} + ${ratio_sum}" | bc`
            else
                # nan
                ratio_sum=`echo "scale=7; ${ratio_sum} + 2" | bc`
            fi
        done

        all=`echo "scale=7; ${all_sum} / ${max_trial}" | bc`
        skipped=`echo "scale=7; ${skipped_sum} / ${max_trial}" | bc`
        ratio=`echo "scale=7; ${ratio_sum} / ${max_trial}" | bc`
        echo "$thr,$all,$skipped,$ratio" >> ${target_dir}/entries_${bench_type}.csv
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
