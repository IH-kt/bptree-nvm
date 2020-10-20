#!/bin/bash

if [ -z "$1" ]; then
    echo "dirname required!"
    exit
fi
scr_dir=../../../src/utility/graph_script/
cd ../../../res/$1/

function plot_elapsed_time() {
    script_dir=$1
    # ${script_dir}/stamp_time_thread_csv.sh
    python3 ${script_dir}/stamp_thread_time.py
    # python3 ${script_dir}/stamp_thread_time_all.py
}

function plot_wait_time() {
    script_dir=$1
    ${script_dir}/stamp_wait_time_thread_csv.sh
    python3 ${script_dir}/stamp_waittime.py
}

function stats() {
    script_dir=$1
    ${script_dir}/stamp_txsize_txratio_writeratio.sh
}

function plot_abort() {
    script_dir=$1
    ${script_dir}/stamp_abort_csv.sh
}

function plot_commit_time() {
    script_dir=$1
    ${script_dir}/stamp_committime_csv.sh
    python3 ${script_dir}/stamp_committime.py .
}

function maxtx() {
    script_dir=$1
    ${script_dir}/stamp_maxtx.sh
}

mkdir -p summary
cd summary
plot_elapsed_time ${scr_dir}
# plot_wait_time ${scr_dir}
# stats ${scr_dir}
# plot_abort ${scr_dir}
# plot_commit_time ${scr_dir}
# maxtx ${scr_dir}
