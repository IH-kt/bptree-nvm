#!/bin/bash
graphscr_dir=../../../src/utility/graph_script/
cd ../../../res/$1/
mkdir -p graph
cd graph
mkdir -p scaling/pmem
mkdir -p scaling/vmem
python3 ${graphscr_dir}/compare_tree_thread_time.py
mkdir -p logsize/pmem
mkdir -p logsize/vmem
python3 ${graphscr_dir}/compare_tree_logsize_time.py

${graphscr_dir}/make_abort_nvhtm_csv.sh ..
python3 ${graphscr_dir}/compare_thread_logsz_nvhtm_abort.py

${graphscr_dir}/make_abort_csv.sh ..
python3 ${graphscr_dir}/compare_thread_logsz_abort.py

${graphscr_dir}/make_write_freq_graph.sh

mkdir -p wait_time/pmem
mkdir -p wait_time/vmem
${graphscr_dir}/make_block_time_logsize_csv.sh ../pmem/elapsed_time
mv logsize_csv wait_time/pmem
${graphscr_dir}/make_block_time_logsize_csv.sh ../vmem/elapsed_time
mv logsize_csv wait_time/vmem
${graphscr_dir}/make_block_time_thread_csv.sh ../pmem/elapsed_time
mv thread_csv wait_time/pmem
${graphscr_dir}/make_block_time_thread_csv.sh ../vmem/elapsed_time
mv thread_csv wait_time/vmem
