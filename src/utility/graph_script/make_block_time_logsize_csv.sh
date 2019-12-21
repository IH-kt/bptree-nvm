#!/bin/sh

if [ $# -eq 0 ]; then
    root_dir=.
else
    root_dir=$1
fi
plain=bptree_nvhtm_0
db=bptree_nvhtm_1
logsz_list=`ls ${root_dir}/${plain} | sed -e "s/logsz_//" | sort -n`
types='plain db'
ops='insert delete search'
max_trial=5
trials=`seq 1 ${max_trial}`
result_dir="logsize_csv"

mkdir -p ${result_dir}
for type in $types
do
    type_dir=`eval echo '$'${type}`
    for op in $ops
    do
        for thr in $thrs
        do
            echo 'logsize,HTM-block,Checkpoint-block,Abort' > ${result_dir}/wait_${op}_${type}.thr.${thr}.csv
            for logsz in $logsz_list
            do
                logsz_dir=logsz_$logsz
                target_dir=`echo "${root_dir}/${type_dir}/${logsz_dir}"`
                thrs=`ls ${target_dir} | grep $op | sed -E 's/[^0-9]+//g' | sort -n`
                dmp=`ls ${target_dir} | grep $op`
                cblock_time_sum=0
                hblock_time_sum=0
                ablock_time_sum=0
                for trial in $trials
                do
                    cblock_time=`cat ${target_dir}/${op}_concurrent.exe.thr.${thr}.dmp | grep "NH time" | cut -f 5 -d ' '`
                    hblock_time=`cat ${target_dir}/${op}_concurrent.exe.thr.${thr}.dmp | grep "HTM " | cut -f 5 -d ' '`
                    ablock_time=`cat ${target_dir}/${op}_concurrent.exe.thr.${thr}.dmp | grep TRANSACTION_ABORT_TIME | cut -f 2 -d ' '`
                    cblock_time_sum=`echo "scale=7; ${cblock_time} + ${cblock_time_sum}" | bc`
                    hblock_time_sum=`echo "scale=7; ${hblock_time} + ${hblock_time_sum}" | bc`
                    ablock_time_sum=`echo "scale=7; ${ablock_time} + ${ablock_time_sum}" | bc`
                done
                cbl_pt=`echo "scale=7; (${cblock_time_sum} / ${max_trial}) / ${thr}" | bc`
                hbl_pt=`echo "scale=7; (${hblock_time_sum} / ${max_trial}) / ${thr}" | bc`
                abl_pt=`echo "scale=7; (${ablock_time_sum} / ${max_trial}) / ${thr}" | bc`
                echo "$thr,$cbl_pt,$hbl_pt,$abl_pt" >> ${result_dir}/wait_${op}_${type}.thr.${thr}.csv
            done
        done
    done
done
