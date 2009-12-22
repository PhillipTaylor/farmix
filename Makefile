all: clean compile link

compile:
	nasm -f elf -o start.o start.asm
	gcc -Wall -O -fstrength-reduce -fleading-underscore -fomit-frame-pointer -finline-functions -nostdinc -fno-builtin -I./include -c -o main.o main.c
	gcc -Wall -O -fstrength-reduce -fleading-underscore -fomit-frame-pointer -finline-functions -nostdinc -fno-builtin -I./include -c -o scrn.o scrn.c
	gcc -Wall -O -fstrength-reduce -fleading-underscore -fomit-frame-pointer -finline-functions -nostdinc -fno-builtin -I./include -c -o gdt.o gdt.c
	gcc -Wall -O -fstrength-reduce -fleading-underscore -fomit-frame-pointer -finline-functions -nostdinc -fno-builtin -I./include -c -o idt.o idt.c
	gcc -Wall -O -fstrength-reduce -fleading-underscore -fomit-frame-pointer -finline-functions -nostdinc -fno-builtin -I./include -c -o isrs.o isrs.c
	gcc -Wall -O -fstrength-reduce -fleading-underscore -fomit-frame-pointer -finline-functions -nostdinc -fno-builtin -I./include -c -o irq.o irq.c
	gcc -Wall -O -fstrength-reduce -fleading-underscore -fomit-frame-pointer -finline-functions -nostdinc -fno-builtin -I./include -c -o timer.o timer.c
	gcc -Wall -O -fstrength-reduce -fleading-underscore -fomit-frame-pointer -finline-functions -nostdinc -fno-builtin -I./include -c -o kb.o kb.c

link:
	ld -T link.ld -o kernel.bin start.o main.o scrn.o gdt.o idt.o isrs.o irq.o timer.o kb.o
	rm -f *.o

active_kernel:
	cp kernel.bin /farmix.bin

clean:
	rm -f *.o
	rm -f kernel.bin
