#!/bin/sh

if [ $# -eq 0 ]; then
    root_dir=..
else
    root_dir=$1
fi
# trees='use_mmap parallel_cp parallel_cp_dram_log log_compression optimized_commit'
trees='use_mmap large_node_use_mmap'
ops='insert delete search mixed'
# max_trial=5
max_trial=3
trials=`seq 1 ${max_trial}`
result_dir="txstats"

function make_csv() {
    src_dir=$1
    tree=$2
    op=$3
    echo 'thread,txsize,txratio,writefreq' > ${result_dir}/$tree/txstats_${op}.csv
    thrs=`ls ${src_dir} | grep $op | cut -f 6 -d '.' | sort -n -u`
    for thr in $thrs
    do
        echo "thr = $thr"
        txsize_sum=0
        txratio_sum=0
        writefreq_sum=0
        for trial in $trials
        do
            write_num=`cat ${src_dir}/${op}_concurrent.exe.cpthr.*.thr.${thr}.trial.${trial}.dmp | grep "TOTAL_WRITES" | cut -f 11 -d ' '`
            tx_num=`cat ${src_dir}/${op}_concurrent.exe.cpthr.*.thr.${thr}.trial.${trial}.dmp | grep "COMMIT" | cut -f 3 -d ' '`
            whole_time=`cat ${src_dir}/${op}_concurrent.exe.cpthr.*.thr.${thr}.trial.${trial}.dmp | tail -n 1`
            tx_time=`cat ${src_dir}/${op}_concurrent.exe.cpthr.*.thr.${thr}.trial.${trial}.dmp | grep "TRANSACTION_TIME" | cut -f 2 -d ' '`
            echo "write_num  = $write_num"
            echo "tx_num     = $tx_num"
            echo "whole_time = $whole_time"
            echo "tx_time    = $tx_time"

            txsize_tmp=`echo "scale=9;$write_num/$tx_num" | bc`
            txratio_tmp=`echo "scale=9;$tx_time/$whole_time" | bc`
            writefreq_tmp=`echo "scale=9;$write_num/$whole_time" | bc`

            txsize_sum=`echo "scale=7; ${txsize_tmp} + ${txsize_sum}" | bc`
            txratio_sum=`echo "scale=7; ${txratio_tmp} + ${txratio_sum}" | bc`
            writefreq_sum=`echo "scale=7; ${writefreq_tmp} + ${writefreq_sum}" | bc`
        done
        txsize=`echo "scale=7; ${txsize_sum} / ${max_trial}" | bc`
        txratio=`echo "scale=7; ${txratio_sum} / ${max_trial}" | bc`
        writefreq=`echo "scale=7; ${writefreq_sum} / ${max_trial}" | bc`
        echo "$thr,$txsize,$txratio,$writefreq" >> ${result_dir}/$tree/txstats_${op}.csv
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
