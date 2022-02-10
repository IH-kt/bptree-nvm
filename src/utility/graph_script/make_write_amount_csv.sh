#!/bin/sh

if [ $# -eq 0 ]; then
    root_dir=.
else
    root_dir=$1
fi
ops='insert delete search'
# types='bptree_concurrent_0 fptree_concurrent_0'
types='fptree_concurrent_0'
# memtypes='pmem vmem'
memtypes='pmem'
max_trial=3
trials=`seq 1 ${max_trial}`

for memtype in $memtypes
do
    for type in $types
    do
        mkdir -p write_amount/$memtype/$type
        for op in $ops
        do
            echo 'type,logsize,thread,worker,checkpoint' > write_amount/$memtype/$type/write_amount_${op}.csv
            target_dir=`echo "${root_dir}/${memtype}/write_amount/${type}/logsz_65824"`
            thrs=`ls ${target_dir} | grep $op | cut -f 4 -d '.' | sort -n -u`
            for thr in $thrs
            do
                    worker_tmp=0
                    for tri in $trials
                    do
                        tmp=`cat ${target_dir}/${op}_concurrent.exe.thr.${thr}.trial.${tri}.dmp | grep "write amount" | cut -f 4 -d ' '`
                        worker_tmp=`echo "scale=7; ${worker_tmp} + ${tmp}" | bc`
                    done
                worker=`echo "scale=7; ${worker_tmp} / ${max_trial}" | bc`
                echo "$type,64824,$thr,$worker,0" >> write_amount/$memtype/$type/write_amount_${op}.csv
            done
        done
    done
done
