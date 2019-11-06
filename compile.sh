#!/bin/sh

ROOT_DIR=`pwd`
BUILD_DIR=$ROOT_DIR/build
TEST_DIR=$ROOT_DIR/src/test
ALLOCATOR_DIR=$ROOT_DIR/src/utility/simple_allocator
SRC_DIR=$ROOT_DIR/src/fptree

FPTREE_SRC=$SRC_DIR/fptree.c
ALLOCATOR_SRC=$ALLOCATOR_DIR/allocator.c
TEST_SRC="simple_test.c insert_test.c search_test.c delete_test.c"
TEST=`echo $TEST_SRC | sed -e 's/\.c/.exe/g'`

mkdir -p $BUILD_DIR

if [ $# -gt 0 ] ; then
    if [ $1 = -u ] ; then
        cp -p $FPTREE_SRC $BUILD_DIR
        cp -p $ALLOCATOR_SRC $BUILD_DIR
        for files in $TEST_SRC
        do
            cp -p $TEST_DIR/$files $BUILD_DIR
        done
        cp -p $ROOT_DIR/Makefile $BUILD_DIR
    elif [ $1 = -d ] ; then
        rm -r $BUILD_DIR
        exit 0
    fi
else
    echo "add -u when sources are updated"
fi

cd $BUILD_DIR

make -j $TEST ROOT_DIR=$ROOT_DIR EXES=$TEST
