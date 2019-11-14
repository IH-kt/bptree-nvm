#!/bin/sh

PMDK_PATH=$HOME/local/

export C_INCLUDE_PATH=$C_INCLUDE_PATH:$PMDK_PATH/include
export CPLUS_INCLUDE_PATH=$CPLUS_INCLUDE_PATH:$PMDK_PATH/include
export LIBRARY_PATH=$LIBRARY_PATH:$PMDK_PATH/lib

if [ ! -e nvhtm-selfcontained/nv-htm/libnh.a ]; then
    cp -R nvhtm_modification_files/* nvhtm-selfcontained/
    (cd nvhtm-selfcontained/nvm-emulation; ./compile.sh)
    (cd nvhtm-selfcontained/htm-alg; ./compile.sh)
	(cd nvhtm-selfcontained/nv-htm; ./compile.sh REDO_TS FORK)
fi

mv nvhtm-selfcontained/nv-htm/libnh.a . 
