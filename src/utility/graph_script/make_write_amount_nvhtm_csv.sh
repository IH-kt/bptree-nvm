#!/bin/sh

if [ $# -eq 0 ]; then
    root_dir=.
else
    root_dir=$1
fi
plain=bptree_nvhtm_0
# db=bptree_nvhtm_1
ca=bptree_nvhtm_1
logsz_list=`ls ${root_dir}/pmem/write_amount/${plain} | sed -e "s/logsz_//" | sort -n`
# types='plain db'
types='plain'
ops='insert delete search'
# memtypes="pmem vmem"
memtypes="pmem"
max_trial=3
trials=`seq 1 ${max_trial}`

for memtype in $memtypes
do
    for type in $types
    do
        type_dir=`eval echo '$'${type}`
        mkdir -p write_amount/$memtype/$type_dir
        for op in $ops
        do
            echo 'type,logsize,thread,worker,checkpoint,nofil' > write_amount/$memtype/$type_dir/write_amount_${op}.csv
            for logsz in $logsz_list
            do
                logsz_dir=logsz_$logsz
                target_dir=`echo "${root_dir}/${memtype}/write_amount/${type_dir}/${logsz_dir}"`
                thrs=`ls ${target_dir} | grep $op | cut -f 4 -d '.' | sort -n -u`
                for thr in $thrs
                do
                    worker_tmp=0
                    checkpoint_tmp=0
                    nofil_tmp=0
                    for tri in $trials
                    do
                        tmp=`grep "write amount (w" ${target_dir}/${op}_concurrent.exe.thr.${thr}.trial.${tri}.dmp | cut -f 4 -d ' '`
                        worker_tmp=`echo "scale=7; ${worker_tmp} + ${tmp}" | bc`
                        tmp=`grep "write amount (c" ${target_dir}/${op}_concurrent.exe.thr.${thr}.trial.${tri}.dmp | cut -f 4 -d ' '`
                        checkpoint_tmp=`echo "scale=7; ${checkpoint_tmp} + ${tmp}" | bc`
                        tmp=`grep "filt" ${target_dir}/${op}_concurrent.exe.thr.${thr}.trial.${tri}.dmp | cut -f 7 -d ' '`
                        nofil_tmp=`echo "scale=7; ${nofil_tmp} + ${tmp}" | bc`
                    done
                    worker=`echo "scale=7; ${worker_tmp} / ${max_trial}" | bc`
                    checkpoint=`echo "scale=7; ${checkpoint_tmp} / ${max_trial}" | bc`
                    nofil=`echo "scale=7; ${nofil_tmp} / ${max_trial}" | bc`
                    echo "$type,$logsz,$thr,$worker,$checkpoint,$nofil" >> write_amount/$memtype/$type_dir/write_amount_${op}.csv
                done
            done
        done
    done
done
