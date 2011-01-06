COMPILER=gcc
BUILD_ARGS= -Wall -O -fstrength-reduce -fomit-frame-pointer -finline-functions -nostdinc -fno-builtin -I./include -c

OS_VERSION="0.1"

PREV_KERNEL_PATH=./prev_kernels
INSTALL_TO=/farmix.bin

all: clean compile link

compile:
	nasm -f elf -o start.o start.asm
	$(COMPILER) $(BUILD_ARGS) -D OS_VERSION='$(OS_VERSION)' -o main.o main.c
	$(COMPILER) $(BUILD_ARGS) -o scrn.o scrn.c
	$(COMPILER) $(BUILD_ARGS) -o mem_utils.o mem_utils.c
	$(COMPILER) $(BUILD_ARGS) -o str_utils.o str_utils.c
	$(COMPILER) $(BUILD_ARGS) -o gdt.o gdt.c
	$(COMPILER) $(BUILD_ARGS) -o idt.o idt.c
	$(COMPILER) $(BUILD_ARGS) -o isrs.o isrs.c
	$(COMPILER) $(BUILD_ARGS) -o irq.o irq.c
	$(COMPILER) $(BUILD_ARGS) -o timer.o timer.c
	$(COMPILER) $(BUILD_ARGS) -o kb.o kb.c

link:
	ld -T link.ld -o kernel.bin mem_utils.o start.o main.o str_utils.o scrn.o gdt.o idt.o isrs.o irq.o timer.o kb.o
	rm -f *.o

active_kernel:
	cp $(INSTALL_TO) $(PREV_KERNEL_PATH)$(INSTALL_TO)_`date +%F_%H_%M`
	cp kernel.bin $(INSTALL_TO)

disassemble: clean compile
	echo -e "disassemble\n\n" > disassemble.asm
	objdump -d *.o >> disassemble.asm
	less disassemble.asm
	rm -f o.*

clean:
	rm -f *.o
	rm -f kernel.bin
