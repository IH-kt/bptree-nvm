#!/bin/sh

if [ $# -eq 0 ]; then
    root_dir=./result_kmeans-high_test_REDO-TS-FORK
else
    root_dir=$1
fi
bench_var='kmeans'
# max_trial=5
max_trial=3
trials=`seq 1 ${max_trial}`
result_dir="$root_dir/block_csv"
thrs="1 4 8"

mkdir -p ${result_dir}
for b in $bench_var
do
    # echo 'thread,HTM-block,Checkpoint-block,Abort,End' > ${result_dir}/wait_${b}.csv
    echo 'thread,HTM-block,Checkpoint-block,Abort' > ${result_dir}/wait_${b}.csv
    target_dir=`echo "${root_dir}/"`
    for thr in $thrs
    do
        cblock_time_sum=0
        hblock_time_sum=0
        ablock_time_sum=0
        end_time_sum=0
        for trial in $trials
        do
            # echo "${target_dir}/${op}_concurrent.exe.thr.${thr}.trial.${trial}.dmp"
            hblock_time=`cat ${target_dir}/result_thr${thr}_trial${trial} | grep "HTM " | cut -f 5 -d ' '`
            # echo "hblock_time = ${hblock_time}"
            cblock_time1=`cat ${target_dir}/result_thr${thr}_trial${trial} | grep "d by WAIT_MORE_LOG" | cut -f 7 -d ' '`
            # echo "cblock_time1 = ${cblock_time1}"
            cblock_time2=`cat ${target_dir}/result_thr${thr}_trial${trial} | grep "d by CHECK_LOG_ABORT" | cut -f 7 -d ' '`
            # echo "cblock_time2 = ${cblock_time2}"
            ablock_time=`cat ${target_dir}/result_thr${thr}_trial${trial} | grep "TRANSACTION_ABORT_TIME" | cut -f 2 -d ' '`
            # echo "ablock_time = ${ablock_time}"
            # end_time=`cat ${target_dir}/result_thr${thr}_trial${trial} | grep "wait_for" | tail -n 1 | cut -f 3 -d ' '`
            # echo "end_time = ${end_time}"
            hblock_time_sum=`echo "scale=7; ${hblock_time} + ${hblock_time_sum}" | bc`
            cblock_time_sum=`echo "scale=7; ${cblock_time1} + ${cblock_time2} + ${cblock_time_sum}" | bc`
            ablock_time_sum=`echo "scale=7; ${ablock_time} + ${ablock_time_sum}" | bc`
            # end_time_sum=`echo "scale=7; ${end_time} + ${end_time_sum}" | bc`
        done
        hbl_pt=`echo "scale=7; (${hblock_time_sum} / ${max_trial}) / ${thr}" | bc`
        cbl_pt=`echo "scale=7; (${cblock_time_sum} / ${max_trial}) / ${thr}" | bc`
        abl_pt=`echo "scale=7; (${ablock_time_sum} / ${max_trial}) / ${thr}" | bc`
        # end_pt=`echo "scale=7; (${end_time_sum} / ${max_trial})" | bc`
        echo "htm = ${hbl_pt}"
        echo "cbl = ${cbl_pt}"
        echo "abl = ${abl_pt}"
        # echo "end = ${end_pt}"
        # echo "$thr,$hbl_pt,$cbl_pt,$abl_pt,$end_pt" >> ${result_dir}/wait_${b}.csv
        echo "$thr,$hbl_pt,$cbl_pt,$abl_pt" >> ${result_dir}/wait_${b}.csv
    done
done
