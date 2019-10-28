#!/bin/sh

ROOT_DIR=`pwd`
BUILD_DIR=$ROOT_DIR/build

FPTREE_SRC=$ROOT_DIR/src/fptree/fptree.c
ALLOCATOR_SRC=$ROOT_DIR/src/utility/simple_allocator/allocator.c
MAIN_SRC=$ROOT_DIR/src/test/simple.c
MAIN=simple.out

mkdir -p $BUILD_DIR

if [ $# -gt 0 ] ; then
    if [ $1 = -u ] ; then
        cp $FPTREE_SRC $BUILD_DIR
        cp $ALLOCATOR_SRC $BUILD_DIR
        cp $MAIN_SRC $BUILD_DIR
        cp $ROOT_DIR/Makefile $BUILD_DIR
    fi
else
    echo "add -u when sources are updated"
fi

cd $BUILD_DIR

make $MAIN ROOT_DIR=$ROOT_DIR
