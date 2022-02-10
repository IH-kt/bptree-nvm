#!/bin/bash
graphscr_dir=../../../src/utility/graph_script/
cd ../../../res/$1/
mkdir -p graph
cd graph

function scaling () {
    python3 ${graphscr_dir}/btree_thread_time.py ..
}

function waittime() {
    ${graphscr_dir}/btree_waittime_csv.sh
    python3 ${graphscr_dir}/btree_waittime.py .
}

function abort() {
    # ${graphscr_dir}/btree_abort_csv.sh
    python3 ${graphscr_dir}/btree_abort.py .
}

function committime() {
    ${graphscr_dir}/btree_committime_csv.sh
    python3 ${graphscr_dir}/btree_committime.py .
}

function maxtx() {
    ${graphscr_dir}/btree_maxtx_csv.sh
}

scaling
# waittime
# abort
# committime
# maxtx
