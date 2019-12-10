BENCH_DATE          ?= $(shell date "+%Y%m%d-%H%M")
BENCH_DIR           := $(RESULT_DIR)/$(BENCH_DATE)
BENCH_WF_DIR        := $(BENCH_DIR)/write_freq
BENCH_ET_DIR        := $(BENCH_DIR)/elapsed_time

ifdef warmup
	WARM_UP	:= $(warmup)
else
	WARM_UP	:= 50000
endif

ifdef loop
	LOOP_TIMES	:= $(loop)
else
	LOOP_TIMES	:= 50000
endif

MAX_VAL	:= $(shell expr $(WARM_UP) + $(LOOP_TIMES))

ifdef logsz
	LOGSZ	:= $(logsz)
else
	LOGSZ	:= 1048576
endif
NVHTM_MAKE_ARGS += LOG_SIZE=$(LOGSZ)
ifdef logmax
	LOGMAX	:= $(logmax)
else
	LOGMAX	:= 12582912
endif
ifdef logstep
	LOGSTP	:= $(logstep)
else
	LOGSTP	:= 4194304
endif
ifdef logstart
	LOGSTT	:= $(logstart)
else
	LOGSTT	:= 4194304
endif

ifdef thrmax
	THRMAX	:= $(thrmax)
else
	THRMAX	:= 22
endif
ifdef thrstep
	THRSTP	:= $(thrstep)
else
	THRSTP	:= 4
endif

ifdef ppath
	PPATH	:= $(ppath)
else
	PPATH	:= .
endif

THRLST	?= $(shell seq 1 $(THRSTP) $(THRMAX) | tr '\n' ' ')
LOGLST	?= $(shell echo "for(i=$(LOGSTT);i<=$(LOGMAX);i+=$(LOGSTP))i" | bc -q | tr '\n' ' ')
THRLSTP	?= [$(shell echo $(THRLST) | tr ' ' ',')]

bench_all:
	make BENCH_DATE=$(BENCH_DATE) ppath=$(PPATH) write_freq
	make BENCH_DATE=$(BENCH_DATE) ppath=$(PPATH) elapsed_time

elapsed_time:
	make tree=fptree type=concurrent ppath=$(PPATH) db=0 thrmax=$(THRMAX) thrstep=$(THRSTP) BENCH_DATE=$(BENCH_DATE) elapsed_time_thr
	make tree=bptree type=nvhtm ppath=$(PPATH) db=0 BENCH_DATE=$(BENCH_DATE) elapsed_time_log 
	make tree=bptree type=nvhtm ppath=$(PPATH) db=1 BENCH_DATE=$(BENCH_DATE) elapsed_time_log

elapsed_time_log:
	for logsz in $(LOGLST); do\
		make tree=$(tree) type=$(type) ppath=$(PPATH) db=$(db) logsz=$$logsz thrmax=$(THRMAX) thrstep=$(THRSTP) BENCH_DATE=$(BENCH_DATE) elapsed_time_thr;\
	done

elapsed_time_thr: bench_et_dir
	make tree=$(tree) type=$(type) dist-clean
	make tree=$(tree) type=$(type) db=$(db) logsz=$(LOGSZ) all
	cp $(BENCH_SCRIPT_DIR)/base_operation.py $(BUILD_DIR)
	(cd build; python3 base_operation.py $(WARM_UP) $(LOOP_TIMES) "$(THRLSTP)" $(PPATH)/data $(PPATH)/log)
	for exe in $(BASE_BENCH_EXE); do\
		for thr in $(THRLST); do\
			mv $(BUILD_DIR)/"$$exe".thr"$$thr".dmp $(BENCH_ET_DIR)/$(tree)_$(type)_$(db)/logsz_$(LOGSZ)/;\
		done;\
	done
	mv $(BUILD_DIR)/result_raw.npy $(BENCH_ET_DIR)/$(tree)_$(type)_$(db)/logsz_$(LOGSZ)/
	mv $(BUILD_DIR)/result.csv $(BENCH_ET_DIR)/$(tree)_$(type)_$(db)/logsz_$(LOGSZ)/

write_freq:
	make tree=fptree type=concurrent db=0 ppath=$(PPATH) thrmax=$(THRMAX) thrstep=$(THRSTP) BENCH_DATE=$(BENCH_DATE) write_freq_thr
	make tree=bptree type=nvhtm db=0 ppath=$(PPATH) BENCH_DATE=$(BENCH_DATE) write_freq_log
	make tree=bptree type=nvhtm db=1 ppath=$(PPATH) BENCH_DATE=$(BENCH_DATE) write_freq_log

write_freq_log:
	for logsz in $(LOGLST); do\
		make tree=$(tree) type=$(type) db=$(db) ppath=$(PPATH) logsz=$$logsz thrmax=$(THRMAX) thrstep=$(THRSTP) BENCH_DATE=$(BENCH_DATE) write_freq_thr;\
	done

write_freq_thr: bench_wf_dir
	make tree=$(tree) type=$(type) fw=1 dist-clean
	make tree=$(tree) type=$(type) db=$(db) fw=1 logsz=$(LOGSZ) all
	cp $(BENCH_SCRIPT_DIR)/write_freq.py $(BUILD_DIR)
	(cd build; python3 write_freq.py $(WARM_UP) $(LOOP_TIMES) "$(THRLSTP)" $(PPATH)/data $(PPATH)/log)
	for thr in $(THRLST); do\
		mv $(BUILD_DIR)/write_freq"$$thr".txt $(BENCH_WF_DIR)/$(tree)_$(type)_$(db)/logsz_$(LOGSZ)/;\
		mv $(BUILD_DIR)/insert_concurrent.exe.thr"$$thr".dmp $(BENCH_WF_DIR)/$(tree)_$(type)_$(db)/logsz_$(LOGSZ)/;\
	done

bench_et_dir:
	mkdir -p $(BENCH_ET_DIR)/$(tree)_$(type)_$(db)/logsz_$(LOGSZ)
bench_wf_dir:
	mkdir -p $(BENCH_WF_DIR)/$(tree)_$(type)_$(db)/logsz_$(LOGSZ)