#!/bin/sh

if [ $# -eq 0 ]; then
    root_dir=.
else
    root_dir=$1
fi
plain=bptree_nvhtm_0
db=bptree_nvhtm_1
logsz_list=`ls ${root_dir}/${plain} | sed -e "s/logsz_//" | sort -n`
# types='plain db'
types='plain'
ops='insert delete search'
max_trial=3
trials=`seq 1 ${max_trial}`
result_dir="thread_csv"

mkdir -p ${result_dir}
for type in $types
do
    type_dir=`eval echo '$'${type}`
    for op in $ops
    do
        for logsz in $logsz_list
        do
            echo 'thread,reserve,commit-finding,log-read,library,logwrite,apply-flush' > ${result_dir}/checkpoint_${op}_${type}.logsize.${logsz}.csv
            logsz_dir=logsz_$logsz
            target_dir=`echo "${root_dir}/${type_dir}/${logsz_dir}"`
            thrs=`ls ${target_dir} | grep $op | cut -f 4 -d '.' | sort -n -u`
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
                    reserve_time=`cat ${target_dir}/${op}_concurrent.exe.thr.${thr}.trial.${trial}.dmp | grep "sec. 1" | cut -f 6 -d ' '`
                    commit_find_time=`cat ${target_dir}/${op}_concurrent.exe.thr.${thr}.trial.${trial}.dmp | grep "sec. 2" | cut -f 7 -d ' '`
                   # acheck_time=`cat ${target_dir}/${op}_concurrent.exe.thr.${thr}.trial.${trial}.dmp | grep "d write" | cut -f 5 -d ' '`
                    logwrite_time=`cat ${target_dir}/${op}_concurrent.exe.thr.${thr}.trial.${trial}.dmp | grep "d lo" | cut -f 5 -d ' '`
                    read_only_time=`cat ${target_dir}/${op}_concurrent.exe.thr.${thr}.trial.${trial}.dmp | grep "d re" | cut -f 5 -d ' '`
                    no_write_time=`cat ${target_dir}/${op}_concurrent.exe.thr.${thr}.trial.${trial}.dmp | grep "d no" | cut -f 5 -d ' '`
                    write_flush_time=`cat ${target_dir}/${op}_concurrent.exe.thr.${thr}.trial.${trial}.dmp | grep "d wr" | cut -f 5 -d ' '`
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
                echo "$thr,$rsrv_time_avg,$cmfd_time_avg,$read_time_avg,$libry_time_diff,$write_time_diff,$wrfl_time_avg" >> ${result_dir}/checkpoint_${op}_${type}.logsize.${logsz}.csv
            done
        done
    done
done
