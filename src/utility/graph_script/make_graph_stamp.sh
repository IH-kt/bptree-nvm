#!/bin/bash

if [ -z "$1" ]; then
    echo "dirname required!"
    exit
fi
scr_dir=../../../src/utility/graph_script/
cd ../../../res/$1/

function plot_elapsed_time() {
    script_dir=$1
    ${script_dir}/stamp_time_thread_csv.sh
    # python3 ${script_dir}/stamp_thread_time.py
    python3 ${script_dir}/stamp_thread_time_grid.py
}

function plot_wait_time() {
    script_dir=$1
    ${script_dir}/stamp_wait_time_thread_csv.sh
    # python3 ${script_dir}/stamp_waittime.py
    ${script_dir}/stamp_wait_time_thread_all_csv.sh
    python3 ${script_dir}/stamp_waittime_all.py
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

function checkpointtime() {
    script_dir=$1
    ${script_dir}/stamp_checkpoint_time_csv.sh
    ${script_dir}/stamp_checkpoint_time_para_csv.sh
    ${script_dir}/stamp_checkpoint_time_logcmp_csv.sh
    # python3 ${script_dir}/stamp_checkpoint_time.py .
    python3 ${script_dir}/stamp_checkpoint_time_all.py .
}

function logentries() {
    script_dir=$1
    echo logentries
    ${script_dir}/stamp_logentries_csv.sh
}

function abortfreq() {
    script_dir=$1
    python3 ${script_dir}/stamp_stats_all.py .
}

function txsize() {
    script_dir=$1
    python3 ${script_dir}/stamp_txsize.py ..
}

mkdir -p summary
cd summary
# plot_elapsed_time ${scr_dir}
# plot_wait_time ${scr_dir}
# stats ${scr_dir}
# plot_abort ${scr_dir}
# checkpointtime ${scr_dir}
# logentries ${scr_dir}
# abortfreq ${scr_dir}
txsize ${scr_dir}

# plot_commit_time ${scr_dir}
# maxtx ${scr_dir}
