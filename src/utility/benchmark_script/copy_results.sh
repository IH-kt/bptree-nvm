#!/bin/bash

result_dir=$1
target_dir=$2

cp $result_dir/*.csv $target_dir
cp $result_dir/*.dmp $target_dir
