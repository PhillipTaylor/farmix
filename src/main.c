#include <system.h>

#ifndef OS_VERSION
	#define OS_VERSION "unspecified"
#endif

unsigned char inportb (unsigned short _port)
{
    unsigned char rv;
    __asm__ __volatile__ ("inb %1, %0" : "=a" (rv) : "dN" (_port));
    return rv;
}

void outportb (unsigned short _port, char _data)
{
    __asm__ __volatile__ ("outb %1, %0" : : "dN" (_port), "a" (_data));
}

void print_welcome()
{
	unsigned long long f = 6799000015ULL;
	unsigned long long *g;
	
	g = &f;

    puts("  Welcome to Farmix!\n");
    puts("   Operating System\n");
    puts("       ");
	puts(OS_VERSION);
	puts("\n");
	
	kprintf("NULL = %i\n", NULL);

	kprintf("testing the new and sexy kprintf: (%x = 15 and %x = 10)\n", 15, 10);

	kprintf("ull test: 1=%U 2=%X 3=%x 4= 5=\n", g, g, g);
}

void _start(void *grub1, unsigned int magic)
{
	int *mem_req = NULL;

    gdt_install();
    idt_install();
    isrs_install();
    irq_install();
    init_video();
	init_memory(grub1, magic);
	//print_memory_map();
	
	//testing malloc
	//mem_req = (int*) malloc(32);
	mem_req = OUT_OF_MEMORY;

	if (mem_req == OUT_OF_MEMORY)
		kprintf("didn't get a usable address back\n");
	else {
		(*mem_req) = 6;
		kprintf("mem_req points to %x\n", mem_req);
		kprintf("mem_req has value of %i\n", *mem_req);
	}

	// reprint memory map
	//print_memory_map();
	print_welcome();
    timer_install();
    keyboard_install();

    __asm__ __volatile__ ("sti");

    for (;;);
}

