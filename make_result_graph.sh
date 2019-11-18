#!/bin/sh

RES_DIR=`ls res`

for res in $RES_DIR
do
    cp src/utility/graph_script/make_graph.py res/$res
    (cd res/$res; python3 make_graph.py; rm make_graph.py)
done