#!/bin/bash
graphscr_dir=../../../src/utility/graph_script/
cd ../../../res/$1/
mkdir -p graph
cd graph
# mkdir -p scaling/pmem
# mkdir -p scaling/vmem
# python3 ${graphscr_dir}/compare_tree_thread_time.py
# mkdir -p logsize/pmem
# mkdir -p logsize/vmem
# python3 ${graphscr_dir}/compare_tree_logsize_time.py
# mv *_result_logsize* logsize
# mkdir -p abort
# ${graphscr_dir}/make_abort_csv.sh ..
# mv pmem vmem abort
# python3 ${graphscr_dir}/compare_thread_logsz_abort.py
${graphscr_dir}/make_write_freq_graph.sh
