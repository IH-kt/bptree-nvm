#!/bin/bash
graphscr_dir=../../../src/utility/graph_script/
cd ../../../res/$1/
mkdir -p graph
cd graph
python3 ${graphscr_dir}/compare_tree_thread_time.py
python3 ${graphscr_dir}/compare_tree_logsize_time.py
${graphscr_dir}/make_abort_csv.sh ../elapsed_time/
python3 ${graphscr_dir}/compare_thread_logsz_abort.py
${graphscr_dir}/make_write_freq_graph.sh
