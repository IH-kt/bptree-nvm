#!/bin/sh

nvhtms="bptree_nvhtm_0 bptree_nvhtm_1"
logszs="33056 65824 131360 262432 524576 1048864"
thrs="1 2 4 8 16"
wf_dir="write_freq"
concs="fptree_concurrent_0 bptree_concurrent_0"
memtypes="pmem vmem"

mkdir -p $wf_dir
for memtype in $memtypes
do
    for nvhtm in $nvhtms
    do
        mkdir -p $wf_dir/$nvhtm
        for ls in $logszs
        do
            mkdir -p $wf_dir/$memtype/$nvhtm/logsz_$ls
            graph_dir=`echo "$wf_dir/$memtype/$nvhtm/logsz_$ls"`
            for thr in $thrs
            do
                cat ../pmem/write_freq/$nvhtm/logsz_${ls}/write_freq${thr}.txt | python3 ../../../src/utility/graph_script/write_freq_graph.py
                mv worker_write_freq.eps $graph_dir/worker_write_freq.$thr.eps
                mv worker_write_freq.png $graph_dir/worker_write_freq.$thr.png
                mv checkpoint_write_freq.eps $graph_dir/checkpoint_write_freq.$thr.eps
                mv checkpoint_write_freq.png $graph_dir/checkpoint_write_freq.$thr.png
            done
        done
    done
    for conc in $concs
    do
        graph_dir=`echo "$wf_dir/$memtype/$conc/logsz_65824"`
        mkdir -p $graph_dir
        for thr in $thrs
        do
            cat ../$memtype/write_freq/$conc/logsz_65824/write_freq${thr}.txt | python3 ../../../src/utility/graph_script/write_freq_graph.py
            mv worker_write_freq.eps $graph_dir/worker_write_freq.$thr.eps
            mv worker_write_freq.png $graph_dir/worker_write_freq.$thr.png
            mv checkpoint_write_freq.eps $graph_dir/checkpoint_write_freq.$thr.eps
            mv checkpoint_write_freq.png $graph_dir/checkpoint_write_freq.$thr.png
        done
    done
done
