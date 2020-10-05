BENCH_DATE          ?= $(shell date "+%Y%m%d-%H%M")
BENCH_DIR           := $(RESULT_DIR)/btrees_$(BENCH_DATE)
SCRIPT_SRC_DIR		:= src/utility/benchmark_script

ifdef warmup
	WARM_UP	:= $(warmup)
else
 	WARM_UP	:= 2500000
endif

ifdef loop
	LOOP_TIMES	:= $(loop)
else
	LOOP_TIMES	:= 2500000
#	LOOP_TIMES	:= 5000000
endif

MAX_VAL	:= $(shell expr $(WARM_UP) + $(LOOP_TIMES))

ifdef logmax
	LOGMAX	:= $(logmax)
else
	LOGMAX	:= 0
endif
ifdef logstep
	LOGSTP	:= $(logstep)
else
	LOGSTP	:= 32768
endif
ifdef logstart
	LOGSTT	:= $(logstart)
else
	LOGSTT	:= 41943328
endif

ifdef thrstart
	THRSTT	:= $(thrstart)
else
	THRSTT	:= 1
endif
ifdef thrend
	THREND	:= $(thrend)
else
	THREND	:= 8
endif

ifdef cpthrstart
	CPTHRSTT	:= $(cpthrstart)
else
	CPTHRSTT	:= 8
endif
ifdef thrend
	CPTHREND	:= $(cpthrend)
else
	CPTHREND	:= 8
endif

ifdef ppath
	PPATH	:= $(ppath)
else
	PPATH	:= .
endif

ifdef vdirpath
	VDPATH  := $(vdirpath)
else
	VDPATH	:= .
endif

ifndef parallel_cp
	parallel_cp := 1
endif

LOGLST	?= $(shell seq 0 $(LOGMAX) | sed -e 's/\(.*\)/$(LOGSTT) * 2^\1 + 288/'| bc -q | tr '\n' ' ')

bench_all:
	(for i in `seq 0 17`; do sudo cpufreq-set -g performance -c $$i; done)
	sudo su -c "echo 1 > /sys/devices/system/cpu/intel_pstate/no_turbo"
	# make BENCH_DATE=$(BENCH_DATE) PPATH=$(PPATH) VDPATH=$(VDPATH) tree=bptree type=nvhtm use_mmap
	# make BENCH_DATE=$(BENCH_DATE) PPATH=$(PPATH) VDPATH=$(VDPATH) tree=bptree type=nvhtm parallel_cp
	# make BENCH_DATE=$(BENCH_DATE) PPATH=$(PPATH) VDPATH=$(VDPATH) tree=bptree type=nvhtm parallel_cp_dram_log
	# make BENCH_DATE=$(BENCH_DATE) PPATH=$(PPATH) VDPATH=$(VDPATH) tree=bptree type=nvhtm log_compression
	# make BENCH_DATE=$(BENCH_DATE) PPATH=$(PPATH) VDPATH=$(VDPATH) tree=bptree type=nvhtm optimized_commit
	# make BENCH_DATE=$(BENCH_DATE) PPATH=$(PPATH) VDPATH=$(VDPATH) tree=bptree type=nvhtm no_fence
	# make BENCH_DATE=$(BENCH_DATE) PPATH=$(PPATH) VDPATH=$(VDPATH) tree=bptree type=nvhtm no_flush
	# make BENCH_DATE=$(BENCH_DATE) PPATH=$(PPATH) VDPATH=$(VDPATH) tree=bptree type=nvhtm no_cp
	# make BENCH_DATE=$(BENCH_DATE) PPATH=$(PPATH) VDPATH=$(VDPATH) tree=bptree type=nvhtm no_cp_dram
	# make BENCH_DATE=$(BENCH_DATE) PPATH=$(PPATH) VDPATH=$(VDPATH) tree=bptree type=nvhtm no_cp_optc
	# make BENCH_DATE=$(BENCH_DATE) PPATH=$(PPATH) VDPATH=$(VDPATH) tree=bptree type=nvhtm large_node_use_mmap
	# make BENCH_DATE=$(BENCH_DATE) PPATH=$(PPATH) VDPATH=$(VDPATH) tree=bptree type=nvhtm large_node_parallel_cp
	# make BENCH_DATE=$(BENCH_DATE) PPATH=$(PPATH) VDPATH=$(VDPATH) tree=bptree type=nvhtm large_node_log_compression
	# make BENCH_DATE=$(BENCH_DATE) PPATH=$(PPATH) VDPATH=$(VDPATH) tree=bptree type=nvhtm more_loop_use_mmap
	# make BENCH_DATE=$(BENCH_DATE) PPATH=$(PPATH) VDPATH=$(VDPATH) tree=bptree type=nvhtm more_loop_parallel_cp 
	# make BENCH_DATE=$(BENCH_DATE) PPATH=$(PPATH) VDPATH=$(VDPATH) tree=bptree type=nvhtm more_loop_log_compression
	# make BENCH_DATE=$(BENCH_DATE) PPATH=$(PPATH) VDPATH=$(VDPATH) tree=bptree type=nvhtm parallel_cp_nctime
	# make BENCH_DATE=$(BENCH_DATE) PPATH=$(PPATH) VDPATH=$(VDPATH) tree=bptree type=nvhtm cp_nothing
	# make BENCH_DATE=$(BENCH_DATE) PPATH=$(PPATH) VDPATH=$(VDPATH) tree=bptree type=nvhtm cp_initialize_only
	# make BENCH_DATE=$(BENCH_DATE) PPATH=$(PPATH) VDPATH=$(VDPATH) tree=bptree type=nvhtm cp_nowrite
	# make BENCH_DATE=$(BENCH_DATE) PPATH=$(PPATH) VDPATH=$(VDPATH) tree=bptree type=nvhtm wait_time_count
	# make BENCH_DATE=$(BENCH_DATE) PPATH=$(PPATH) VDPATH=$(VDPATH) tree=bptree type=nvhtm cp_noflush
	# make BENCH_DATE=$(BENCH_DATE) PPATH=$(PPATH) VDPATH=$(VDPATH) tree=bptree type=nvhtm ex_small_leaf_use_mmap
	make BENCH_DATE=$(BENCH_DATE) PPATH=$(PPATH) VDPATH=$(VDPATH) tree=bptree type=nvhtm ex_maxtx_use_mmap
	make BENCH_DATE=$(BENCH_DATE) PPATH=$(PPATH) VDPATH=$(VDPATH) tree=bptree type=nvhtm ex_maxtx_small_leaf_use_mmap
	make BENCH_DATE=$(BENCH_DATE) PPATH=$(PPATH) VDPATH=$(VDPATH) tree=bptree type=nvhtm ex_maxtx_small_leaf_noinit_use_mmap
	(for i in `seq 0 17`; do sudo cpufreq-set -g powersave -c $$i; done)

use_mmap:
	mkdir -p $(BENCH_DIR)/$@
	make tree=$(tree) type=$(type) build_use_mmap
	python3 $(SCRIPT_SRC_DIR)/base_operation.py $(BUILD_DIR) $(WARM_UP) $(LOOP_TIMES) $(THRSTT) $(THREND) 1 1 $(PPATH)/data $(PPATH)/log
	$(SCRIPT_SRC_DIR)/copy_results.sh $(BUILD_DIR) $(BENCH_DIR)/$@

use_mmap_dram_log:
	mkdir -p $(BENCH_DIR)/$@
	make tree=$(tree) type=$(type) build_use_mmap
	python3 $(SCRIPT_SRC_DIR)/base_operation.py $(BUILD_DIR) $(WARM_UP) $(LOOP_TIMES) $(THRSTT) $(THREND) 1 1 $(PPATH)/data $(VDPATH)/log
	$(SCRIPT_SRC_DIR)/copy_results.sh $(BUILD_DIR) $(BENCH_DIR)/$@

parallel_cp:
	mkdir -p $(BENCH_DIR)/$@
	make tree=$(tree) type=$(type) build_parallel_cp
	python3 $(SCRIPT_SRC_DIR)/base_operation.py $(BUILD_DIR) $(WARM_UP) $(LOOP_TIMES) $(THRSTT) $(THREND) $(CPTHRSTT) $(CPTHREND) $(PPATH)/data $(PPATH)/log
	$(SCRIPT_SRC_DIR)/copy_results.sh $(BUILD_DIR) $(BENCH_DIR)/$@

parallel_cp_dram_log:
	mkdir -p $(BENCH_DIR)/$@
	make tree=$(tree) type=$(type) build_parallel_cp
	python3 $(SCRIPT_SRC_DIR)/base_operation.py $(BUILD_DIR) $(WARM_UP) $(LOOP_TIMES) $(THRSTT) $(THREND) $(CPTHRSTT) $(CPTHREND) $(PPATH)/data $(VDPATH)/log
	$(SCRIPT_SRC_DIR)/copy_results.sh $(BUILD_DIR) $(BENCH_DIR)/$@

log_compression:
	mkdir -p $(BENCH_DIR)/$@
	make tree=$(tree) type=$(type) build_parallel_cp
	python3 $(SCRIPT_SRC_DIR)/base_operation.py $(BUILD_DIR) $(WARM_UP) $(LOOP_TIMES) $(THRSTT) $(THREND) $(CPTHRSTT) $(CPTHREND) $(PPATH)/data $(PPATH)/log
	$(SCRIPT_SRC_DIR)/copy_results.sh $(BUILD_DIR) $(BENCH_DIR)/$@

log_compression_dram_log:
	mkdir -p $(BENCH_DIR)/$@
	make tree=$(tree) type=$(type) build_parallel_cp
	python3 $(SCRIPT_SRC_DIR)/base_operation.py $(BUILD_DIR) $(WARM_UP) $(LOOP_TIMES) $(THRSTT) $(THREND) $(CPTHRSTT) $(CPTHREND) $(PPATH)/data $(VDPATH)/log
	$(SCRIPT_SRC_DIR)/copy_results.sh $(BUILD_DIR) $(BENCH_DIR)/$@

number_of_entries:
	mkdir -p $(BENCH_DIR)/$@
	make tree=$(tree) type=$(type) entries=$(entries) build_log_compression
	python3 $(SCRIPT_SRC_DIR)/base_operation.py $(BUILD_DIR) $(WARM_UP) $(LOOP_TIMES) $(THRSTT) $(THREND) $(CPTHRSTT) $(CPTHREND) $(PPATH)/data $(PPATH)/log
	$(SCRIPT_SRC_DIR)/copy_results.sh $(BUILD_DIR) $(BENCH_DIR)/$@

measure_cp_parallel_cp:
	mkdir -p $(BENCH_DIR)/$@
	make tree=$(tree) type=$(type) measure_cp=1 build_parallel_cp
	python3 $(SCRIPT_SRC_DIR)/base_operation.py $(BUILD_DIR) $(WARM_UP) $(LOOP_TIMES) $(THRSTT) $(THREND) $(CPTHRSTT) $(CPTHREND) $(PPATH)/data $(PPATH)/log
	$(SCRIPT_SRC_DIR)/copy_results.sh $(BUILD_DIR) $(BENCH_DIR)/$@

measure_cp_log_compression:
	mkdir -p $(BENCH_DIR)/$@
	make tree=$(tree) type=$(type) measure_cp=1 build_log_compression
	python3 $(SCRIPT_SRC_DIR)/base_operation.py $(BUILD_DIR) $(WARM_UP) $(LOOP_TIMES) $(THRSTT) $(THREND) $(CPTHRSTT) $(CPTHREND) $(PPATH)/data $(PPATH)/log
	$(SCRIPT_SRC_DIR)/copy_results.sh $(BUILD_DIR) $(BENCH_DIR)/$@

optimized_commit:
	mkdir -p $(BENCH_DIR)/$@
	make tree=$(tree) type=$(type) opt_c=1 build_log_compression
	python3 $(SCRIPT_SRC_DIR)/base_operation.py $(BUILD_DIR) $(WARM_UP) $(LOOP_TIMES) $(THRSTT) $(THREND) $(CPTHRSTT) $(CPTHREND) $(PPATH)/data $(PPATH)/log
	$(SCRIPT_SRC_DIR)/copy_results.sh $(BUILD_DIR) $(BENCH_DIR)/$@

no_fence:
	mkdir -p $(BENCH_DIR)/$@
	make tree=$(tree) type=$(type) no_fence=1 build_log_compression
	python3 $(SCRIPT_SRC_DIR)/base_operation.py $(BUILD_DIR) $(WARM_UP) $(LOOP_TIMES) $(THRSTT) $(THREND) $(CPTHRSTT) $(CPTHREND) $(PPATH)/data $(PPATH)/log
	$(SCRIPT_SRC_DIR)/copy_results.sh $(BUILD_DIR) $(BENCH_DIR)/$@

no_flush:
	mkdir -p $(BENCH_DIR)/$@
	make tree=$(tree) type=$(type) no_flush=1 build_log_compression
	python3 $(SCRIPT_SRC_DIR)/base_operation.py $(BUILD_DIR) $(WARM_UP) $(LOOP_TIMES) $(THRSTT) $(THREND) $(CPTHRSTT) $(CPTHREND) $(PPATH)/data $(PPATH)/log
	$(SCRIPT_SRC_DIR)/copy_results.sh $(BUILD_DIR) $(BENCH_DIR)/$@

no_cp:
	mkdir -p $(BENCH_DIR)/$@
	make tree=$(tree) type=$(type) no_cp=1 build_use_mmap
	python3 $(SCRIPT_SRC_DIR)/base_operation.py $(BUILD_DIR) $(WARM_UP) $(LOOP_TIMES) $(THRSTT) $(THREND) $(CPTHRSTT) $(CPTHREND) $(PPATH)/data $(PPATH)/log
	$(SCRIPT_SRC_DIR)/copy_results.sh $(BUILD_DIR) $(BENCH_DIR)/$@

no_cp_dram:
	mkdir -p $(BENCH_DIR)/$@
	make tree=$(tree) type=$(type) no_cp=1 build_use_mmap
	python3 $(SCRIPT_SRC_DIR)/base_operation.py $(BUILD_DIR) $(WARM_UP) $(LOOP_TIMES) $(THRSTT) $(THREND) $(CPTHRSTT) $(CPTHREND) $(PPATH)/data $(VDPATH)/log
	$(SCRIPT_SRC_DIR)/copy_results.sh $(BUILD_DIR) $(BENCH_DIR)/$@

no_cp_optc:
	mkdir -p $(BENCH_DIR)/$@
	make tree=$(tree) type=$(type) opt_c=1 no_cp=1 build_use_mmap
	python3 $(SCRIPT_SRC_DIR)/base_operation.py $(BUILD_DIR) $(WARM_UP) $(LOOP_TIMES) $(THRSTT) $(THREND) $(CPTHRSTT) $(CPTHREND) $(PPATH)/data $(PPATH)/log
	$(SCRIPT_SRC_DIR)/copy_results.sh $(BUILD_DIR) $(BENCH_DIR)/$@

large_node_use_mmap:
	mkdir -p $(BENCH_DIR)/$@
	make tree=$(tree) type=$(type) leafsz=128 nodesz=128 build_use_mmap
	python3 $(SCRIPT_SRC_DIR)/base_operation.py $(BUILD_DIR) $(WARM_UP) $(LOOP_TIMES) $(THRSTT) $(THREND) $(CPTHRSTT) $(CPTHREND) $(PPATH)/data $(PPATH)/log
	$(SCRIPT_SRC_DIR)/copy_results.sh $(BUILD_DIR) $(BENCH_DIR)/$@

large_node_parallel_cp:
	mkdir -p $(BENCH_DIR)/$@
	make tree=$(tree) type=$(type) leafsz=128 nodesz=128 build_parallel_cp
	python3 $(SCRIPT_SRC_DIR)/base_operation.py $(BUILD_DIR) $(WARM_UP) $(LOOP_TIMES) $(THRSTT) $(THREND) $(CPTHRSTT) $(CPTHREND) $(PPATH)/data $(PPATH)/log
	$(SCRIPT_SRC_DIR)/copy_results.sh $(BUILD_DIR) $(BENCH_DIR)/$@

large_node_log_compression:
	mkdir -p $(BENCH_DIR)/$@
	make tree=$(tree) type=$(type) leafsz=128 nodesz=128 build_log_compression
	python3 $(SCRIPT_SRC_DIR)/base_operation.py $(BUILD_DIR) $(WARM_UP) $(LOOP_TIMES) $(THRSTT) $(THREND) $(CPTHRSTT) $(CPTHREND) $(PPATH)/data $(PPATH)/log
	$(SCRIPT_SRC_DIR)/copy_results.sh $(BUILD_DIR) $(BENCH_DIR)/$@

more_loop_use_mmap:
	mkdir -p $(BENCH_DIR)/$@
	make tree=$(tree) type=$(type) build_use_mmap
	python3 $(SCRIPT_SRC_DIR)/base_operation.py $(BUILD_DIR) $(WARM_UP) 5000000 $(THRSTT) $(THREND) $(CPTHRSTT) $(CPTHREND) $(PPATH)/data $(PPATH)/log
	$(SCRIPT_SRC_DIR)/copy_results.sh $(BUILD_DIR) $(BENCH_DIR)/$@

more_loop_parallel_cp:
	mkdir -p $(BENCH_DIR)/$@
	make tree=$(tree) type=$(type) build_parallel_cp
	python3 $(SCRIPT_SRC_DIR)/base_operation.py $(BUILD_DIR) $(WARM_UP) 5000000 $(THRSTT) $(THREND) $(CPTHRSTT) $(CPTHREND) $(PPATH)/data $(PPATH)/log
	$(SCRIPT_SRC_DIR)/copy_results.sh $(BUILD_DIR) $(BENCH_DIR)/$@

more_loop_log_compression:
	mkdir -p $(BENCH_DIR)/$@
	make tree=$(tree) type=$(type) build_log_compression
	python3 $(SCRIPT_SRC_DIR)/base_operation.py $(BUILD_DIR) $(WARM_UP) 5000000 $(THRSTT) $(THREND) $(CPTHRSTT) $(CPTHREND) $(PPATH)/data $(PPATH)/log
	$(SCRIPT_SRC_DIR)/copy_results.sh $(BUILD_DIR) $(BENCH_DIR)/$@

parallel_cp_nctime:
	mkdir -p $(BENCH_DIR)/$@
	make tree=$(tree) type=$(type) nctime=1 build_parallel_cp
	python3 $(SCRIPT_SRC_DIR)/base_operation.py $(BUILD_DIR) $(WARM_UP) $(LOOP_TIMES) $(THRSTT) $(THREND) $(CPTHRSTT) $(CPTHREND) $(PPATH)/data $(PPATH)/log
	$(SCRIPT_SRC_DIR)/copy_results.sh $(BUILD_DIR) $(BENCH_DIR)/$@

cp_nothing:
	mkdir -p $(BENCH_DIR)/$@
	make tree=$(tree) type=$(type) cpnt=1 build_use_mmap
	python3 $(SCRIPT_SRC_DIR)/base_operation.py $(BUILD_DIR) $(WARM_UP) $(LOOP_TIMES) $(THRSTT) $(THREND) $(CPTHRSTT) $(CPTHREND) $(PPATH)/data $(PPATH)/log
	$(SCRIPT_SRC_DIR)/copy_results.sh $(BUILD_DIR) $(BENCH_DIR)/$@

cp_initialize_only:
	mkdir -p $(BENCH_DIR)/$@
	make tree=$(tree) type=$(type) cpit=1 build_use_mmap
	python3 $(SCRIPT_SRC_DIR)/base_operation.py $(BUILD_DIR) $(WARM_UP) $(LOOP_TIMES) $(THRSTT) $(THREND) $(CPTHRSTT) $(CPTHREND) $(PPATH)/data $(PPATH)/log
	$(SCRIPT_SRC_DIR)/copy_results.sh $(BUILD_DIR) $(BENCH_DIR)/$@

cp_nowrite:
	mkdir -p $(BENCH_DIR)/$@
	make tree=$(tree) type=$(type) cpnw=1 build_use_mmap
	python3 $(SCRIPT_SRC_DIR)/base_operation.py $(BUILD_DIR) $(WARM_UP) $(LOOP_TIMES) $(THRSTT) $(THREND) $(CPTHRSTT) $(CPTHREND) $(PPATH)/data $(PPATH)/log
	$(SCRIPT_SRC_DIR)/copy_results.sh $(BUILD_DIR) $(BENCH_DIR)/$@

cp_noflush:
	mkdir -p $(BENCH_DIR)/$@
	make tree=$(tree) type=$(type) cpnf=1 build_use_mmap
	python3 $(SCRIPT_SRC_DIR)/base_operation.py $(BUILD_DIR) $(WARM_UP) $(LOOP_TIMES) $(THRSTT) $(THREND) $(CPTHRSTT) $(CPTHREND) $(PPATH)/data $(PPATH)/log
	$(SCRIPT_SRC_DIR)/copy_results.sh $(BUILD_DIR) $(BENCH_DIR)/$@

wait_time_count:
	mkdir -p $(BENCH_DIR)/$@
	make tree=$(tree) type=$(type) nwt=1 build_use_mmap
	python3 $(SCRIPT_SRC_DIR)/base_operation.py $(BUILD_DIR) $(WARM_UP) $(LOOP_TIMES) $(THRSTT) $(THREND) $(CPTHRSTT) $(CPTHREND) $(PPATH)/data $(PPATH)/log
	$(SCRIPT_SRC_DIR)/copy_results.sh $(BUILD_DIR) $(BENCH_DIR)/$@

ex_small_leaf_use_mmap:
	mkdir -p $(BENCH_DIR)/$@
	make tree=$(tree) type=$(type) leafsz=3 build_use_mmap
	python3 $(SCRIPT_SRC_DIR)/base_operation.py $(BUILD_DIR) $(WARM_UP) $(LOOP_TIMES) $(THRSTT) $(THREND) $(CPTHRSTT) $(CPTHREND) $(PPATH)/data $(PPATH)/log
	$(SCRIPT_SRC_DIR)/copy_results.sh $(BUILD_DIR) $(BENCH_DIR)/$@

ex_maxtx_use_mmap:
	mkdir -p $(BENCH_DIR)/$@
	make tree=$(tree) type=$(type) max_tx_size=1 build_use_mmap
	python3 $(SCRIPT_SRC_DIR)/base_operation.py $(BUILD_DIR) $(WARM_UP) $(LOOP_TIMES) $(THRSTT) $(THREND) $(CPTHRSTT) $(CPTHREND) $(PPATH)/data $(PPATH)/log
	$(SCRIPT_SRC_DIR)/copy_results.sh $(BUILD_DIR) $(BENCH_DIR)/$@

ex_maxtx_small_leaf_use_mmap:
	mkdir -p $(BENCH_DIR)/$@
	make tree=$(tree) type=$(type) leafsz=3 max_tx_size=1 build_use_mmap
	python3 $(SCRIPT_SRC_DIR)/base_operation.py $(BUILD_DIR) $(WARM_UP) $(LOOP_TIMES) $(THRSTT) $(THREND) $(CPTHRSTT) $(CPTHREND) $(PPATH)/data $(PPATH)/log
	$(SCRIPT_SRC_DIR)/copy_results.sh $(BUILD_DIR) $(BENCH_DIR)/$@

ex_maxtx_small_leaf_noinit_use_mmap:
	mkdir -p $(BENCH_DIR)/$@
	make tree=$(tree) type=$(type) leafsz=3 no_init_leaf=1 max_tx_size=1 build_use_mmap
	python3 $(SCRIPT_SRC_DIR)/base_operation.py $(BUILD_DIR) $(WARM_UP) $(LOOP_TIMES) $(THRSTT) $(THREND) $(CPTHRSTT) $(CPTHREND) $(PPATH)/data $(PPATH)/log
	$(SCRIPT_SRC_DIR)/copy_results.sh $(BUILD_DIR) $(BENCH_DIR)/$@

build_use_mmap:
	make tree=$(tree) type=$(type) dist-clean
	make tree=$(tree) type=$(type) logsize=$(LOGSZ) stats=1 no_flush=$(no_flush) no_fence=$(no_fence) no_cp=$(no_cp) opt_c=$(opt_c) nctime=$(nctime) -j all

build_parallel_cp:
	make tree=$(tree) type=$(type) dist-clean
	make tree=$(tree) type=$(type) logsize=$(LOGSZ) stats=1 parallel_cp=1 measure_cp=$(measure_cp) no_flush=$(no_flush) no_fence=$(no_fence) no_cp=$(no_cp) nctime=$(nctime) -j all

build_log_compression:
	make tree=$(tree) type=$(type) dist-clean
	make tree=$(tree) type=$(type) logsize=$(LOGSZ) stats=1 parallel_cp=1 log_compression=1 measure_cp=$(measure_cp) entries=$(entries) opt_c=$(opt_c) no_flush=$(no_flush) no_fence=$(no_fence) no_cp=$(no_cp) nctime=$(nctime) -j all
