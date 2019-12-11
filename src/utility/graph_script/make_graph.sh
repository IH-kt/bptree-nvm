#!/bin/bash
cd ../../../res/20191210-0832/
mkdir -p graph
cd graph
python3 ../../../src/utility/graph_script/compare_tree_thread_time.py
python3 ../../../src/utility/graph_script/compare_tree_logsize_time.py
