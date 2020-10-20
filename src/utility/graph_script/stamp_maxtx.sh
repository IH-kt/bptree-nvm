#!/bin/sh

if [ $# -eq 0 ]; then
    root_dir=..
else
    root_dir=$1
fi
# exec_types='emulator log_comp para_cp use_mmap'
# exec_types='log_comp para_cp use_mmap'
exec_types='max_tx_use_mmap'
bench_types='genome intruder kmeans-high kmeans-low labyrinth ssca2 vacation-high vacation-low yada'
max_trial=3
trials=`seq 1 ${max_trial}`
result_dir="./maxtx"
thrs="1 2 4 8"

function calc_maxtx() {
    target_dir=$1
    bench_type=$2
    maxtx=`cat $target_dir/result_thr1_trial1 | grep "max tx." | cut -f 5 -d ' '`
    echo $maxtx
}

function add_csv () {
    bench_type=$1
    source_dir=$2
    target_csv=$3

    echo $bench_type
    thr=1
    source_dir_tmp=`echo "$source_dir/$bench_type"`
    maxtx=`calc_maxtx $source_dir_tmp $bench_type`
    echo "$bench_type,$maxtx" >> ${target_csv}
}

function apply_all_bench () {
    exec_type=$1
    root_dir=$2
    result_dir=$3
    mkdir -p $result_dir/$exec_type
    echo 'bench_type,maxtx' > ${result_dir}/${exec_type}/stats.csv
    for bench_type in $bench_types
    do
        add_csv $bench_type $root_dir/test_$exec_type $result_dir/$exec_type/stats.csv
    done
}

mkdir -p ${result_dir}
for exec_type in $exec_types
do
    echo $exec_type
    apply_all_bench $exec_type $root_dir $result_dir
done
