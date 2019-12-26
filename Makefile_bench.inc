BENCH_DATE          ?= $(shell date "+%Y%m%d-%H%M")
BENCH_DIR           := $(RESULT_DIR)/$(BENCH_DATE)
BENCH_CA_DIR        := $(BENCH_DIR)/count_abort
BENCH_WF_DIR        := $(BENCH_DIR)/write_freq
BENCH_ET_DIR        := $(BENCH_DIR)/elapsed_time

ifdef warmup
	WARM_UP	:= $(warmup)
else
	WARM_UP	:= 500000
endif

ifdef loop
	LOOP_TIMES	:= $(loop)
else
	LOOP_TIMES	:= 500000
endif

MAX_VAL	:= $(shell expr $(WARM_UP) + $(LOOP_TIMES))

ifdef logsz
	LOGSZ	:= $(logsz)
else
	LOGSZ	:= 65824
endif
NVHTM_MAKE_ARGS += LOG_SIZE=$(LOGSZ)
ifdef logmax
	LOGMAX	:= $(logmax)
else
	LOGMAX	:= 5
endif
ifdef logstep
	LOGSTP	:= $(logstep)
else
	LOGSTP	:= 32768
endif
ifdef logstart
	LOGSTT	:= $(logstart)
else
	LOGSTT	:= 2621440
endif

ifdef thrmax
	THRMAX	:= $(thrmax)
else
	THRMAX	:= 4
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

THRLST	?= $(shell seq 0 $(THRMAX) | sed -e 's/\(.*\)/2^\1/' | bc -q | tr '\n' ' ')
LOGLST	?= $(shell seq 0 $(LOGMAX) | sed -e 's/\(.*\)/$(LOGSTT) * 2^\1 + 288/'| bc -q | tr '\n' ' ')
THRLSTP	?= [$(shell echo $(THRLST) | tr ' ' ',')]
TRIALS  ?= `seq 1 5`

bench_all:
	mkdir -p $(BENCH_DIR)/pmem $(BENCH_DIR)/vmem
	make BENCH_DATE=$(BENCH_DATE) ppath=$(VDPATH) tree=fptree type=concurrent count_abort
	make BENCH_DATE=$(BENCH_DATE) ppath=$(VDPATH) write_freq
	make BENCH_DATE=$(BENCH_DATE) ppath=$(VDPATH) elapsed_time
	mv $(BENCH_ET_DIR) $(BENCH_CA_DIR) $(BENCH_WF_DIR) $(BENCH_DIR)/vmem
	make BENCH_DATE=$(BENCH_DATE) ppath=$(PPATH) tree=bptree type=concurrent count_abort
	make BENCH_DATE=$(BENCH_DATE) ppath=$(PPATH) tree=fptree type=concurrent count_abort
	make BENCH_DATE=$(BENCH_DATE) ppath=$(PPATH) write_freq
	make BENCH_DATE=$(BENCH_DATE) ppath=$(PPATH) elapsed_time
	mv $(BENCH_ET_DIR) $(BENCH_CA_DIR) $(BENCH_WF_DIR) $(BENCH_DIR)/pmem

# for concurrent (not nvhtm)
count_abort: bench_ca_dir
	make tree=$(tree) type=$(type) dist-clean
	make tree=$(tree) type=$(type) ca=1 -j all
	cp $(BENCH_SCRIPT_DIR)/base_operation.py $(BUILD_DIR)
	(cd build; python3 base_operation.py $(WARM_UP) $(LOOP_TIMES) "$(THRLSTP)" $(PPATH)/data $(PPATH)/log)
	for exe in $(BASE_BENCH_EXE); do\
		for thr in $(THRLST); do\
            for trial in $(TRIALS); do\
                mv $(BUILD_DIR)/"$$exe".thr."$$thr".trial."$$trial".dmp $(BENCH_CA_DIR)/$(tree)_$(type)_$(db)/logsz_$(LOGSZ)/;\
                done;\
		done;\
	done
	mv $(BUILD_DIR)/result_raw.npy $(BENCH_CA_DIR)/$(tree)_$(type)_$(db)/logsz_$(LOGSZ)/
	mv $(BUILD_DIR)/result.csv $(BENCH_CA_DIR)/$(tree)_$(type)_$(db)/logsz_$(LOGSZ)/

elapsed_time:
	make tree=fptree type=concurrent ppath=$(PPATH) db=0 thrmax=$(THRMAX) BENCH_DATE=$(BENCH_DATE) elapsed_time_thr
	make tree=bptree type=concurrent ppath=$(PPATH) db=0 thrmax=$(THRMAX) BENCH_DATE=$(BENCH_DATE) elapsed_time_thr
    # make tree=bptree type=nvhtm ppath=$(PPATH) db=1 BENCH_DATE=$(BENCH_DATE) elapsed_time_log
	make tree=bptree type=nvhtm ppath=$(PPATH) db=0 BENCH_DATE=$(BENCH_DATE) elapsed_time_log 

elapsed_time_log:
	for logsz in $(LOGLST); do\
		make tree=$(tree) type=$(type) ppath=$(PPATH) db=$(db) logsz=$$logsz thrmax=$(THRMAX) BENCH_DATE=$(BENCH_DATE) elapsed_time_thr;\
	done

elapsed_time_thr: bench_et_dir
	make tree=$(tree) type=$(type) dist-clean
	make tree=$(tree) type=$(type) db=$(db) logsz=$(LOGSZ) -j all
	cp $(BENCH_SCRIPT_DIR)/base_operation.py $(BUILD_DIR)
	(cd build; python3 base_operation.py $(WARM_UP) $(LOOP_TIMES) "$(THRLSTP)" $(PPATH)/data $(PPATH)/log)
	for exe in $(BASE_BENCH_EXE); do\
		for thr in $(THRLST); do\
            for trial in $(TRIALS); do\
                mv $(BUILD_DIR)/"$$exe".thr."$$thr".trial."$$trial".dmp $(BENCH_ET_DIR)/$(tree)_$(type)_$(db)/logsz_$(LOGSZ)/;\
            done;\
		done;\
	done
	mv $(BUILD_DIR)/result_raw.npy $(BENCH_ET_DIR)/$(tree)_$(type)_$(db)/logsz_$(LOGSZ)/
	mv $(BUILD_DIR)/result.csv $(BENCH_ET_DIR)/$(tree)_$(type)_$(db)/logsz_$(LOGSZ)/

write_freq:
	make tree=fptree type=concurrent db=0 ppath=$(PPATH) thrmax=$(THRMAX) BENCH_DATE=$(BENCH_DATE) write_freq_thr
	make tree=bptree type=concurrent db=0 ppath=$(PPATH) thrmax=$(THRMAX) BENCH_DATE=$(BENCH_DATE) write_freq_thr
    # make tree=bptree type=nvhtm db=1 ppath=$(PPATH) BENCH_DATE=$(BENCH_DATE) write_freq_log
	make tree=bptree type=nvhtm db=0 ppath=$(PPATH) BENCH_DATE=$(BENCH_DATE) write_freq_log

write_freq_log:
	for logsz in $(LOGLST); do\
		make tree=$(tree) type=$(type) db=$(db) ppath=$(PPATH) logsz=$$logsz thrmax=$(THRMAX) BENCH_DATE=$(BENCH_DATE) write_freq_thr;\
	done

write_freq_thr: bench_wf_dir
	make tree=$(tree) type=$(type) fw=1 dist-clean
	make tree=$(tree) type=$(type) db=$(db) fw=1 logsz=$(LOGSZ) -j all
	cp $(BENCH_SCRIPT_DIR)/write_freq.py $(BUILD_DIR)
	(cd build; python3 write_freq.py $(WARM_UP) $(LOOP_TIMES) "$(THRLSTP)" $(PPATH)/data $(PPATH)/log)
	for thr in $(THRLST); do\
        mv $(BUILD_DIR)/write_freq"$$thr".txt $(BENCH_WF_DIR)/$(tree)_$(type)_$(db)/logsz_$(LOGSZ)/;\
        mv $(BUILD_DIR)/insert_concurrent.exe.thr."$$thr".dmp $(BENCH_WF_DIR)/$(tree)_$(type)_$(db)/logsz_$(LOGSZ)/;\
	done

bench_ca_dir:
	mkdir -p $(BENCH_CA_DIR)/$(tree)_$(type)_$(db)/logsz_$(LOGSZ)
bench_et_dir:
	mkdir -p $(BENCH_ET_DIR)/$(tree)_$(type)_$(db)/logsz_$(LOGSZ)
bench_wf_dir:
	mkdir -p $(BENCH_WF_DIR)/$(tree)_$(type)_$(db)/logsz_$(LOGSZ)
