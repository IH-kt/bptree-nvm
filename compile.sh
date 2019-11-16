#!/bin/sh


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

	cp -p $ROOT_DIR/libnh.a $BUILD_DIR
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
NVHTM=-DNVHTM
# ALLOCATOR=
make -j ROOT_DIR=$ROOT_DIR EXES="$BASE_BENCH $TEST" FPTREE_SRC=$FPTREE_SRC_NAME ALLOCATOR_SRC=$ALLOCATOR_SRC_NAME THREAD_MANAGER_SRC=$THREAD_MANAGER_SRC_NAME TIME_PART=$TIME_PART ALLOCATOR=$ALLOCATOR NVHTM=$NVHTM
