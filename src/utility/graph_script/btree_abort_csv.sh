#!/bin/sh

if [ $# -eq 0 ]; then
    root_dir=..
else
    root_dir=$1
fi
trees='use_mmap parallel_cp parallel_cp_dram_log log_compression optimized_commit no_flush no_fence'
ops='insert delete search mixed'
# max_trial=5
max_trial=3
trials=`seq 1 ${max_trial}`
result_dir="abort"

function make_csv() {
    src_dir=$1
    tree=$2
    op=$3
    echo 'thread,abort,conflict,capacity,explicit' > $result_dir/$tree/abort_${op}.csv
    thrs=`ls ${src_dir} | grep $op | cut -f 6 -d '.' | sort -n -u`
    for thr in $thrs
    do
        abort_tmp=0
        conflict_tmp=0
        capacity_tmp=0
        explicit_tmp=0
        for tri in $trials
        do
            tmp=`grep "ABORTS :" ${src_dir}/${op}_concurrent.exe.cpthr.*.thr.${thr}.trial.${tri}.dmp | cut -f 3 -d ' '`
            abort_tmp=`echo "scale=7; ${abort_tmp} + ${tmp}" | bc`
            tmp=`grep "CONFLS :" ${src_dir}/${op}_concurrent.exe.cpthr.*.thr.${thr}.trial.${tri}.dmp | cut -f 3 -d ' '`
            conflict_tmp=`echo "scale=7; ${conflict_tmp} + ${tmp}" | bc`
            tmp=`grep "CAPACS :" ${src_dir}/${op}_concurrent.exe.cpthr.*.thr.${thr}.trial.${tri}.dmp | cut -f 3 -d ' '`
            capacity_tmp=`echo "scale=7; ${capacity_tmp} + ${tmp}" | bc`
            tmp=`grep "EXPLIC :" ${src_dir}/${op}_concurrent.exe.cpthr.*.thr.${thr}.trial.${tri}.dmp | cut -f 3 -d ' '`
            explicit_tmp=`echo "scale=7; ${explicit_tmp} + ${tmp}" | bc`
        done
        abort=`echo "scale=7; ${abort_tmp} / ${max_trial}" | bc`
        conflict=`echo "scale=7; ${conflict_tmp} / ${max_trial}" | bc`
        capacity=`echo "scale=7; ${capacity_tmp} / ${max_trial}" | bc`
        explicit=`echo "scale=7; ${explicit_tmp} / ${max_trial}" | bc`
        echo "$thr,$abort,$conflict,$capacity,$explicit" >> $result_dir/$tree/abort_${op}.csv
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
