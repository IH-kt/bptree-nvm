#!/bin/sh

if [ $# -eq 0 ]; then
    root_dir=..
else
    root_dir=$1
fi
# trees='use_mmap parallel_cp parallel_cp_dram_log log_compression optimized_commit more_loop_use_mmap more_loop_parallel_cp more_loop_log_compression'
# trees='parallel_cp parallel_cp_dram_log log_compression optimized_commit'
# trees='parallel_cp parallel_cp_dram_log no_cp'
trees='cp_noflush'
# trees='cp_initialize_only cp_nothing cp_nowrite'
# trees='cp_initialize_only cp_nothing cp_nowrite no_cp no_cp_dram no_cp_optc'
# trees='optimized_commit no_cp_optc'
ops='insert delete search mixed'
# max_trial=5
max_trial=3
trials=`seq 1 ${max_trial}`
result_dir="committime"

function make_csv() {
    src_dir=$1
    tree=$2
    op=$3
    echo 'thread,clwb,otherwait,fence' > ${result_dir}/$tree/commit_${op}.csv
    thrs=`ls ${src_dir} | grep $op | cut -f 6 -d '.' | sort -n -u`
    for thr in $thrs
    do
        clwb_sum=0
        otherwait_sum=0
        fence_sum=0
        for trial in $trials
        do
            clwb=`cat ${src_dir}/${op}_concurrent.exe.cpthr.*.thr.${thr}.trial.${trial}.dmp | grep "COMMIT_TIME1:" | cut -f 2 -d ' '`
            otherwait=`cat ${src_dir}/${op}_concurrent.exe.cpthr.*.thr.${thr}.trial.${trial}.dmp | grep "COMMIT_TIME2:" | cut -f 2 -d ' '`
            fence=`cat ${src_dir}/${op}_concurrent.exe.cpthr.*.thr.${thr}.trial.${trial}.dmp | grep "COMMIT_TIME3:" | cut -f 2 -d ' '`
            # echo "end_time = ${end_time}"
            clwb_sum=`echo "scale=7; ${clwb} + ${clwb_sum}" | bc`
            otherwait_sum=`echo "scale=7; ${otherwait} + ${otherwait_sum}" | bc`
            fence_sum=`echo "scale=7; ${fence} + ${fence_sum}" | bc`
        done
        clwb_thr=`echo "scale=7; (${clwb_sum} / ${max_trial}) / ${thr}" | bc`
        otherwait_thr=`echo "scale=7; (${otherwait_sum} / ${max_trial}) / ${thr}" | bc`
        fence_thr=`echo "scale=7; (${fence_sum} / ${max_trial}) / ${thr}" | bc`
        echo "$thr,$clwb_thr,$otherwait_thr,$fence_thr" >> ${result_dir}/$tree/commit_${op}.csv
    done
}

function all_ops () {
    src_dir=$1
    tree=$2
    echo $tree
    for op in $ops
    do
        make_csv $src_dir $tree $op
    done
}

function all_trees () {
    for tree in $trees
    do
        mkdir -p $result_dir/$tree
        all_ops $root_dir/$tree $tree
    done
}

all_trees
