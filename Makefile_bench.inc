BENCH_DATE          ?= $(shell date "+%Y%m%d-%H%M")
BENCH_DIR           := $(RESULT_DIR)/$(BENCH_DATE)
BENCH_CA_DIR        := $(BENCH_DIR)/count_abort
BENCH_WF_DIR        := $(BENCH_DIR)/write_freq
BENCH_ET_DIR        := $(BENCH_DIR)/elapsed_time
BENCH_WA_DIR        := $(BENCH_DIR)/write_amount

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
	LOGMAX	:= 3
endif
ifdef logstep
	LOGSTP	:= $(logstep)
else
	LOGSTP	:= 32768
endif
ifdef logstart
	LOGSTT	:= $(logstart)
else
	LOGSTT	:= 10485760
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

ifndef reduce
	reduce := 0
endif

ifndef faw_cp
	faw_cp	:= 0
endif

THRLST	?= $(shell seq 0 $(THRMAX) | sed -e 's/\(.*\)/2^\1/' | bc -q | tr '\n' ' ')
LOGLST	?= $(shell seq 0 $(LOGMAX) | sed -e 's/\(.*\)/$(LOGSTT) * 2^\1 + 288/'| bc -q | tr '\n' ' ')
THRLSTP	?= [$(shell echo $(THRLST) | tr ' ' ',')]
TRIALS  ?= `seq 1 3`

bench_all:
	mkdir -p $(BENCH_DIR)/pmem $(BENCH_DIR)/vmem
	# make BENCH_DATE=$(BENCH_DATE) ppath=$(VDPATH) tree=fptree type=concurrent count_abort
	# make BENCH_DATE=$(BENCH_DATE) ppath=$(VDPATH) write_freq
	# make BENCH_DATE=$(BENCH_DATE) ppath=$(VDPATH) elapsed_time
	# make BENCH_DATE=$(BENCH_DATE) ppath=$(VDPATH) reduce=1 elapsed_time
	# mv $(BENCH_ET_DIR) $(BENCH_CA_DIR) $(BENCH_WF_DIR) $(BENCH_DIR)/vmem
	# mv $(BENCH_ET_DIR) $(BENCH_DIR)/vmem
	# mv $(BENCH_WF_DIR) $(BENCH_DIR)/vmem
	# make BENCH_DATE=$(BENCH_DATE) ppath=$(PPATH) tree=bptree type=concurrent count_abort
	# make BENCH_DATE=$(BENCH_DATE) ppath=$(PPATH) tree=fptree type=concurrent count_abort
	# make BENCH_DATE=$(BENCH_DATE) ppath=$(PPATH) write_freq
	make BENCH_DATE=$(BENCH_DATE) ppath=$(PPATH) elapsed_time
	# make BENCH_DATE=$(BENCH_DATE) ppath=$(PPATH) faw_cp=1 elapsed_time
	# make BENCH_DATE=$(BENCH_DATE) ppath=$(PPATH) reduce=1 elapsed_time
	# mv $(BENCH_ET_DIR) $(BENCH_CA_DIR) $(BENCH_WF_DIR) $(BENCH_DIR)/pmem
	mv $(BENCH_ET_DIR) $(BENCH_DIR)/pmem
	# mv $(BENCH_WF_DIR) $(BENCH_DIR)/pmem
	# make BENCH_DATE=$(BENCH_DATE) ppath=$(PPATH) write_amount
	# mv $(BENCH_WA_DIR) $(BENCH_DIR)/pmem
	# make tree=bptree type=nvhtm logsz=41943328 BENCH_DATE=$(BENCH_DATE) thrmax=$(THRMAX) ppath=$(PPATH) vpath=$(VDPATH) elapsed_time_thr_hetero
	# mv $(BENCH_ET_DIR) $(BENCH_DIR)/pmem

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

write_amount:
	# make tree=fptree type=concurrent ppath=$(PPATH) db=0 thrmax=$(THRMAX) BENCH_DATE=$(BENCH_DATE) write_amount=1 write_amount_thr
	make tree=bptree type=nvhtm logsz=41943328 ppath=$(PPATH) db=0 thrmax=$(THRMAX) BENCH_DATE=$(BENCH_DATE) write_amount_nvhtm=1 write_amount_thr

write_amount_thr: bench_wa_dir
	make tree=$(tree) type=$(type) dist-clean
	make tree=$(tree) type=$(type) db=$(db) logsz=$(LOGSZ) stats=1 reduce=$(reduce) write_amount=$(write_amount) write_amount_nvhtm=$(write_amount_nvhtm) -j all
	cp $(BENCH_SCRIPT_DIR)/base_operation.py $(BUILD_DIR)
	(cd build; python3 base_operation.py $(WARM_UP) $(LOOP_TIMES) "$(THRLSTP)" $(PPATH)/data $(PPATH)/log)
	for exe in $(BASE_BENCH_EXE); do\
		for thr in $(THRLST); do\
			for trial in $(TRIALS); do\
				mv $(BUILD_DIR)/"$$exe".thr."$$thr".trial."$$trial".dmp $(BENCH_WA_DIR)/$(tree)_$(type)_$(db)/logsz_$(LOGSZ)/;\
			done;\
		done;\
	done
	mv $(BUILD_DIR)/result_raw.npy $(BENCH_WA_DIR)/$(tree)_$(type)_$(db)/logsz_$(LOGSZ)/
	mv $(BUILD_DIR)/result.csv $(BENCH_WA_DIR)/$(tree)_$(type)_$(db)/logsz_$(LOGSZ)/


elapsed_time:
	# make tree=fptree type=concurrent ppath=$(PPATH) db=0 thrmax=$(THRMAX) BENCH_DATE=$(BENCH_DATE) elapsed_time_thr
	# make tree=bptree type=concurrent ppath=$(PPATH) db=0 thrmax=$(THRMAX) BENCH_DATE=$(BENCH_DATE) elapsed_time_thr
	make tree=bptree type=nvhtm db=0 logsz=41943328 ppath=$(PPATH) thrmax=$(THRMAX) BENCH_DATE=$(BENCH_DATE) reduce=$(reduce) faw_cp=$(faw_cp) elapsed_time_thr
	# make tree=bptree type=nvhtm ppath=$(PPATH) db=1 BENCH_DATE=$(BENCH_DATE) elapsed_time_log
	# make tree=bptree type=nvhtm ppath=$(PPATH) db=0 BENCH_DATE=$(BENCH_DATE) elapsed_time_log 

elapsed_time_log:
	for logsz in $(LOGLST); do\
		make tree=$(tree) type=$(type) ppath=$(PPATH) db=$(db) logsz=$$logsz thrmax=$(THRMAX) BENCH_DATE=$(BENCH_DATE) elapsed_time_thr;\
	done

elapsed_time_thr: bench_et_dir
	make tree=$(tree) type=$(type) dist-clean
	make tree=$(tree) type=$(type) db=$(db) logsz=$(LOGSZ) stats=1 reduce=$(reduce) faw_cp=$(faw_cp) -j all
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

elapsed_time_thr_hetero: bench_et_ht_dir
	make tree=$(tree) type=$(type) dist-clean
	make tree=$(tree) type=$(type) db=$(db) logsz=$(LOGSZ) stats=1 reduce=$(reduce) faw_cp=$(faw_cp) -j all
	cp $(BENCH_SCRIPT_DIR)/base_operation.py $(BUILD_DIR)
	(cd build; python3 base_operation.py $(WARM_UP) $(LOOP_TIMES) "$(THRLSTP)" $(ppath)/data $(vpath)/log)
	for exe in $(BASE_BENCH_EXE); do\
		for thr in $(THRLST); do\
			for trial in $(TRIALS); do\
				mv $(BUILD_DIR)/"$$exe".thr."$$thr".trial."$$trial".dmp $(BENCH_ET_DIR)/$(tree)_$(type)_usr_pmem/logsz_$(LOGSZ)/;\
			done;\
		done;\
	done
	mv $(BUILD_DIR)/result_raw.npy $(BENCH_ET_DIR)/$(tree)_$(type)_usr_pmem/logsz_$(LOGSZ)/
	mv $(BUILD_DIR)/result.csv $(BENCH_ET_DIR)/$(tree)_$(type)_usr_pmem/logsz_$(LOGSZ)/
	(cd build; python3 base_operation.py $(WARM_UP) $(LOOP_TIMES) "$(THRLSTP)" $(vpath)/data $(ppath)/log)
	for exe in $(BASE_BENCH_EXE); do\
		for thr in $(THRLST); do\
			for trial in $(TRIALS); do\
				mv $(BUILD_DIR)/"$$exe".thr."$$thr".trial."$$trial".dmp $(BENCH_ET_DIR)/$(tree)_$(type)_log_pmem/logsz_$(LOGSZ)/;\
			done;\
		done;\
	done
	mv $(BUILD_DIR)/result_raw.npy $(BENCH_ET_DIR)/$(tree)_$(type)_log_pmem/logsz_$(LOGSZ)/
	mv $(BUILD_DIR)/result.csv $(BENCH_ET_DIR)/$(tree)_$(type)_log_pmem/logsz_$(LOGSZ)/
	

write_freq:
	# make tree=fptree type=concurrent db=0 ppath=$(PPATH) thrmax=$(THRMAX) BENCH_DATE=$(BENCH_DATE) write_freq_thr
	# make tree=bptree type=concurrent db=0 ppath=$(PPATH) thrmax=$(THRMAX) BENCH_DATE=$(BENCH_DATE) write_freq_thr
	make tree=bptree type=nvhtm db=0 logsz=41943328 ppath=$(PPATH) thrmax=$(THRMAX) BENCH_DATE=$(BENCH_DATE) write_freq_thr
	# make tree=bptree type=nvhtm db=1 ppath=$(PPATH) BENCH_DATE=$(BENCH_DATE) write_freq_log
	# make tree=bptree type=nvhtm db=0 ppath=$(PPATH) BENCH_DATE=$(BENCH_DATE) write_freq_log
	# make tree=bptree type=nvhtm chkpa=1 ppath=$(PPATH) BENCH_DATE=$(BENCH_DATE) write_freq_log

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
bench_et_ht_dir:
	mkdir -p $(BENCH_ET_DIR)/$(tree)_$(type)_log_pmem/logsz_$(LOGSZ)
	mkdir -p $(BENCH_ET_DIR)/$(tree)_$(type)_usr_pmem/logsz_$(LOGSZ)
bench_wf_dir:
	mkdir -p $(BENCH_WF_DIR)/$(tree)_$(type)_$(db)/logsz_$(LOGSZ)
bench_wa_dir:
	mkdir -p $(BENCH_WA_DIR)/$(tree)_$(type)_$(db)/logsz_$(LOGSZ)
