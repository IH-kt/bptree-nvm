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

for type in $types
do
    type_dir=`eval echo '$'${type}`
    for op in $ops
    do
        echo 'type,logsize,thread,abort,conflict,capacity,explicit' > abort_${op}_${type}.csv
        for logsz in $logsz_list
        do
            logsz_dir=logsz_$logsz
            target_dir=`echo "${root_dir}/${type_dir}/${logsz_dir}"`
            thrs=`ls ${target_dir} | grep $op | sed -E 's/[^0-9]+//g' | sort -n`
            dmp=`ls ${target_dir} | grep $op`
            for thr in $thrs
            do
                abort=`grep "ABORTS :" ${target_dir}/${op}_concurrent.exe.thr${thr}.dmp | cut -f 3 -d ' '`
                conflict=`grep "CONFLS :" ${target_dir}/${op}_concurrent.exe.thr${thr}.dmp | cut -f 3 -d ' '`
                capacity=`grep "CAPACS :" ${target_dir}/${op}_concurrent.exe.thr${thr}.dmp | cut -f 3 -d ' '`
                explicit=`grep "EXPLIC :" ${target_dir}/${op}_concurrent.exe.thr${thr}.dmp | cut -f 3 -d ' '`
                echo "$type,$logsz,$thr,$abort,$conflict,$capacity,$explicit" >> abort_${op}_${type}.csv
            done
        done
    done
done
