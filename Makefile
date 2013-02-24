COMPILER=gcc
SHELL=sh

OS_VERSION="0.1"

BUILD_DIR=./build
SRC_DIR=./src
PREV_KERNEL_PATH=./prev_kernels
INSTALL_TO=/farmix.bin

BUILD_ARGS= -Wall -O -fstrength-reduce -fomit-frame-pointer -finline-functions -nostdinc -fno-builtin -I./$(SRC_DIR)/include -c

all: link

compile: clean
	mkdir $(BUILD_DIR)
	nasm -f elf -o $(BUILD_DIR)/start.o $(SRC_DIR)/start.asm
	$(COMPILER) $(BUILD_ARGS) -D OS_VERSION='$(OS_VERSION)' -o $(BUILD_DIR)/main.o $(SRC_DIR)/main.c
	$(COMPILER) $(BUILD_ARGS) -o $(BUILD_DIR)/scrn.o $(SRC_DIR)/scrn.c
	$(COMPILER) $(BUILD_ARGS) -o $(BUILD_DIR)/mem_utils.o $(SRC_DIR)/memory/mem_utils.c
	$(COMPILER) $(BUILD_ARGS) -o $(BUILD_DIR)/mem_detect.o $(SRC_DIR)/memory/mem_detect.c
	$(COMPILER) $(BUILD_ARGS) -o $(BUILD_DIR)/mem_api.o $(SRC_DIR)/memory/mem_api.c
	$(COMPILER) $(BUILD_ARGS) -o $(BUILD_DIR)/str_utils.o $(SRC_DIR)/str_utils.c
	$(COMPILER) $(BUILD_ARGS) -o $(BUILD_DIR)/gdt.o $(SRC_DIR)/gdt.c
	$(COMPILER) $(BUILD_ARGS) -o $(BUILD_DIR)/idt.o $(SRC_DIR)/idt.c
	$(COMPILER) $(BUILD_ARGS) -o $(BUILD_DIR)/isrs.o $(SRC_DIR)/isrs.c
	$(COMPILER) $(BUILD_ARGS) -o $(BUILD_DIR)/irq.o $(SRC_DIR)/irq.c
	$(COMPILER) $(BUILD_ARGS) -o $(BUILD_DIR)/timer.o $(SRC_DIR)/timer.c
	$(COMPILER) $(BUILD_ARGS) -o $(BUILD_DIR)/kb.o $(SRC_DIR)/kb.c
	$(COMPILER) $(BUILD_ARGS) -o $(BUILD_DIR)/ramdisk.o $(SRC_DIR)/ramdisk.c

OBJECT_FILES := $(BUILD_DIR)/mem_utils.o
OBJECT_FILES := $(OBJECT_FILES) $(BUILD_DIR)/mem_api.o
OBJECT_FILES := $(OBJECT_FILES) $(BUILD_DIR)/mem_detect.o
OBJECT_FILES := $(OBJECT_FILES) $(BUILD_DIR)/start.o
OBJECT_FILES := $(OBJECT_FILES) $(BUILD_DIR)/main.o
OBJECT_FILES := $(OBJECT_FILES) $(BUILD_DIR)/str_utils.o
OBJECT_FILES := $(OBJECT_FILES) $(BUILD_DIR)/scrn.o
OBJECT_FILES := $(OBJECT_FILES) $(BUILD_DIR)/gdt.o
OBJECT_FILES := $(OBJECT_FILES) $(BUILD_DIR)/idt.o
OBJECT_FILES := $(OBJECT_FILES) $(BUILD_DIR)/isrs.o
OBJECT_FILES := $(OBJECT_FILES) $(BUILD_DIR)/irq.o
OBJECT_FILES := $(OBJECT_FILES) $(BUILD_DIR)/timer.o
OBJECT_FILES := $(OBJECT_FILES) $(BUILD_DIR)/kb.o
OBJECT_FILES := $(OBJECT_FILES) $(BUILD_DIR)/ramdisk.o

link: compile
	ld -T $(SRC_DIR)/link.ld -o $(BUILD_DIR)/kernel.bin $(OBJECT_FILES)

clean: prev_kernel
	rm -rf $(BUILD_DIR)

prev_kernel:
	cp $(INSTALL_TO) $(PREV_KERNEL_PATH)$(INSTALL_TO)_`date +%y-%m-%d_%H_%M` || true

active_kernel:
	cp $(BUILD_DIR)/kernel.bin $(INSTALL_TO)

disassemble: compile
	objdump -d $(OBJECT_FILES) >> $(BUILD_DIR)/disassemble.asm
	less $(BUILD_DIR)/disassemble.asm

