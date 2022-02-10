#!/bin/sh

if [ $# -eq 0 ]; then
    root_dir=..
else
    root_dir=$1
fi
trees='use_mmap parallel_cp parallel_cp_dram_log log_compression optimized_commit'
ops='insert delete search mixed'
# max_trial=5
max_trial=3
trials=`seq 1 ${max_trial}`
result_dir="waittime"

function make_csv() {
    src_dir=$1
    tree=$2
    op=$3
    echo 'thread,HTM-block,Checkpoint-block,Abort,End' > ${result_dir}/$tree/wait_${op}.csv
    thrs=`ls ${src_dir} | grep $op | cut -f 6 -d '.' | sort -n -u`
    for thr in $thrs
    do
        cblock_time_sum=0
        hblock_time_sum=0
        ablock_time_sum=0
        end_time_sum=0
        for trial in $trials
        do
            # echo "${src_dir}/${op}_concurrent.exe.thr.${thr}.trial.${trial}.dmp"
            hblock_time=`cat ${src_dir}/${op}_concurrent.exe.cpthr.*.thr.${thr}.trial.${trial}.dmp | grep "HTM " | cut -f 5 -d ' '`
            # echo "hblock_time = ${hblock_time}"
            cblock_time1=`cat ${src_dir}/${op}_concurrent.exe.cpthr.*.thr.${thr}.trial.${trial}.dmp | grep "d by WAIT_MORE_LOG" | cut -f 7 -d ' '`
            # echo "cblock_time1 = ${cblock_time1}"
            cblock_time2=`cat ${src_dir}/${op}_concurrent.exe.cpthr.*.thr.${thr}.trial.${trial}.dmp | grep "d by CHECK_LOG_ABORT" | cut -f 7 -d ' '`
            # echo "cblock_time2 = ${cblock_time2}"
            ablock_time=`cat ${src_dir}/${op}_concurrent.exe.cpthr.*.thr.${thr}.trial.${trial}.dmp | grep "TRANSACTION_ABORT_TIME" | cut -f 2 -d ' '`
            # echo "ablock_time = ${ablock_time}"
            end_time=`cat ${src_dir}/${op}_concurrent.exe.cpthr.*.thr.${thr}.trial.${trial}.dmp | grep "wait_for" | tail -n 1 | cut -f 3 -d ' '`
            # echo "end_time = ${end_time}"
            hblock_time_sum=`echo "scale=7; ${hblock_time} + ${hblock_time_sum}" | bc`
            cblock_time_sum=`echo "scale=7; ${cblock_time1} + ${cblock_time2} + ${cblock_time_sum}" | bc`
            ablock_time_sum=`echo "scale=7; ${ablock_time} + ${ablock_time_sum}" | bc`
            end_time_sum=`echo "scale=7; ${end_time} + ${end_time_sum}" | bc`
        done
        hbl_pt=`echo "scale=7; (${hblock_time_sum} / ${max_trial}) / ${thr}" | bc`
        cbl_pt=`echo "scale=7; (${cblock_time_sum} / ${max_trial}) / ${thr}" | bc`
        abl_pt=`echo "scale=7; (${ablock_time_sum} / ${max_trial}) / ${thr}" | bc`
        end_pt=`echo "scale=7; (${end_time_sum} / ${max_trial})" | bc`
        # echo "htm = ${hbl_pt}"
        # echo "cbl = ${cbl_pt}"
        # echo "abl = ${abl_pt}"
        # echo "end = ${end_pt}"
        echo "$thr,$hbl_pt,$cbl_pt,$abl_pt,$end_pt" >> ${result_dir}/$tree/wait_${op}.csv
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
