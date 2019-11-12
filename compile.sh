#!/bin/sh

ROOT_DIR=`pwd`
BUILD_DIR=$ROOT_DIR/build
SRC_DIR=$ROOT_DIR/src
TEST_DIR=$SRC_DIR/test
BENCH_DIR=$SRC_DIR/benchmark
BASE_BENCH_DIR=$BENCH_DIR/base_operation
# ALLOCATOR_DIR=$SRC_DIR/utility/simple_allocator
ALLOCATOR_DIR=$SRC_DIR/utility/multithread_allocator
FPTREE_DIR=$SRC_DIR/fptree
THREAD_MANAGER_DIR=$SRC_DIR/utility/thread_manager
BENCH_SCRIPT_DIR=$SRC_DIR/utility/benchmark_script

FPTREE_SRC_NAME=fptree_concurrent.c
ALLOCATOR_SRC_NAME=allocator.c
THREAD_MANAGER_SRC_NAME=thread_manager.c
BENCH_SCRIPT_SRC_NAME=base_operation.py

FPTREE_SRC=$FPTREE_DIR/$FPTREE_SRC_NAME
ALLOCATOR_SRC=$ALLOCATOR_DIR/$ALLOCATOR_SRC_NAME
THREAD_MANAGER_SRC=$THREAD_MANAGER_DIR/$THREAD_MANAGER_SRC_NAME
BENCH_SCRIPT_SRC=$BENCH_SCRIPT_DIR/$BENCH_SCRIPT_SRC_NAME
TEST_SRC="simple_test.c insert_test.c search_test.c delete_test.c thread_test.c"
TEST=`echo $TEST_SRC | sed -e 's/\.c/.exe/g'`
BASE_BENCH_SRC="insert_concurrent.c search_concurrent.c delete_concurrent.c"
BASE_BENCH=`echo $BASE_BENCH_SRC | sed -e 's/\.c/.exe/g'`

mkdir -p $BUILD_DIR

if [ ! -e $BUILD_DIR/Makefile ] || ([ $# -gt 0 ] && [ "$1" = "-u" ]) ; then
	cp -p $FPTREE_SRC $BUILD_DIR
	cp -p $ALLOCATOR_SRC $BUILD_DIR
	cp -p $THREAD_MANAGER_SRC $BUILD_DIR
	for files in $TEST_SRC
	do
		cp -p $TEST_DIR/$files $BUILD_DIR
	done
	for files in $BASE_BENCH_SRC
	do
		cp -p $BASE_BENCH_DIR/$files $BUILD_DIR
	done
	cp -p $BENCH_SCRIPT_SRC $BUILD_DIR
	cp -p $ROOT_DIR/Makefile $BUILD_DIR

	if [ ! -e $ROOT_DIR/libnh.a ]; then
		$ROOT_DIR/make_nv-htm_lib.sh
	fi

	cp $ROOT_DIR/libnh.a $BUILD_DIR
elif [ $# -gt 0 ] ; then
	if [ "$1" = "-d" ] ; then
		rm -r $BUILD_DIR
		exit 0
	fi
fi

if [ $# -eq 0 ] ; then
	echo "add -u when sources are updated"
fi

cd $BUILD_DIR

# TIME_PART=-DTIME_PART
TIME_PART=
ALLOCATOR=-DMT_ALLOCATOR
# ALLOCATOR=
make -j ROOT_DIR=$ROOT_DIR EXES="$BASE_BENCH" FPTREE_SRC=$FPTREE_SRC_NAME ALLOCATOR_SRC=$ALLOCATOR_SRC_NAME THREAD_MANAGER_SRC=$THREAD_MANAGER_SRC_NAME TIME_PART=$TIME_PART ALLOCATOR=$ALLOCATOR
