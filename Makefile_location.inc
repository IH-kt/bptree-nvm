ROOT_DIR    := .
BUILD_DIR   := $(ROOT_DIR)/build
SRC_DIR     := $(ROOT_DIR)/src
INCLUDE_DIR := $(ROOT_DIR)/include

SIMPLE      := simple
CONCURRENT  := concurrent
NVHTM       := nvhtm

ifndef type
    TYPE    := $(SIMPLE)
else
	TYPE	:= $(type)
endif

TEST_SRC_DIR        := $(SRC_DIR)/test
BENCH_SRC_DIR		:= $(SRC_DIR)/benchmark
BASE_BENCH_SRC_DIR	:= $(BENCH_SRC_DIR)/base_operation
UTILITY_DIR         := $(SRC_DIR)/utility
FPTREE_DIR          := $(SRC_DIR)/fptree
ALLOCATOR_DIR       := $(UTILITY_DIR)/allocator
THREAD_MANAGER_DIR  := $(UTILITY_DIR)/thread_manager
BENCH_SCRIPT_DIR    := $(UTILITY_DIR)/benchmark_script

FPTREE_SRC_NAME         := fptree.c
ALLOCATOR_SRC_NAME      := allocator.c
THREAD_MANAGER_SRC_NAME := thread_manager.c
BENCH_SCRIPT_SRC_NAME   := base_operation.py
TEST_SRC_NAME			:= $(shell ls $(TEST_SRC_DIR)/$(TYPE))
BASE_BENCH_SRC_NAME		:= $(shell ls $(BASE_BENCH_SRC_DIR))

FPTREE_SRC          := $(FPTREE_DIR)/$(TYPE)/$(FPTREE_SRC_NAME)
ALLOCATOR_SRC       := $(ALLOCATOR_DIR)/$(TYPE)/$(ALLOCATOR_SRC_NAME)
THREAD_MANAGER_SRC  := $(THREAD_MANAGER_DIR)/$(TYPE)/$(THREAD_MANAGER_SRC_NAME)
BENCH_SCRIPT_SRC    := $(BENCH_SCRIPT_DIR)/$(BENCH_SCRIPT_SRC_NAME)
TEST_SRC 			:= $(addprefix $(TEST_SRC_DIR)/$(TYPE)/, $(TEST_SRC_NAME))
BASE_BENCH_SRC 		:= $(addprefix $(BASE_BENCH_SRC_DIR)/$(TYPE)/, $(BASE_BENCH_SRC_NAME))
