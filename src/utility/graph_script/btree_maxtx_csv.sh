#!/bin/sh

if [ $# -eq 0 ]; then
    root_dir=..
else
    root_dir=$1
fi
# trees='use_mmap parallel_cp parallel_cp_dram_log log_compression optimized_commit'
trees='ex_maxtx_use_mmap ex_maxtx_small_leaf_use_mmap ex_maxtx_small_leaf_noinit_use_mmap'
ops='insert delete search mixed'
# max_trial=5
max_trial=3
trials=`seq 1 ${max_trial}`
result_dir="maxtx"

function make_csv() {
    src_dir=$1
    tree=$2
    op=$3
    echo 'thread,maxtx' > ${result_dir}/$tree/maxtx_${op}.csv
    thrs=`ls ${src_dir} | grep $op | cut -f 6 -d '.' | sort -n -u`
    for thr in $thrs
    do
        echo "thr = $thr"
        maxtx_sum=0
        for trial in $trials
        do
            maxtx=`cat ${src_dir}/${op}_concurrent.exe.cpthr.*.thr.${thr}.trial.${trial}.dmp | grep "max tx." | cut -f 5 -d ' '`


            maxtx_sum=`echo "scale=7; ${maxtx} + ${maxtx_sum}" | bc`
        done
        maxtx=`echo "scale=7; ${maxtx_sum} / ${max_trial}" | bc`
        echo "$thr,$maxtx" >> ${result_dir}/$tree/maxtx_${op}.csv
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
