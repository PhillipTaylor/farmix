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

	puts("    888888888888888888888888888888888888888888888888888888888888888888888  \n");
	puts("    888888888888888888888888888888888888888888888888888888888888888888888  \n");

	puts("     Phillip Ross Taylor's                                                 \n");

	puts("     88888888888                                          88               \n");
	puts("     88                                                   ""               \n");
	puts("     88                                                                    \n");
	puts("     88aaaaa  ,adPPYYba,  8b,dPPYba,  88,dPYba,,adPYba,   88  8b,     ,d8  \n");
	puts("     88\"\"\"\"\"  \"\"     `Y8  88P'   \"Y8  88P'   \"88\"    \"8a  88   `Y8, ,8P'   \n");
	puts("     88       ,adPPPPP88  88          88      88      88  88     )888(     \n");
	puts("     88       88,    ,88  88          88      88      88  88   ,d8\" \"8b,   \n");
	puts("     88       `\"8bbdP\"Y8  88          88      88      88  88  8P'     `Y8  \n");

	puts("                                                         Operating System  \n");

	puts("    888888888888888888888888888888888888888888888888888888888888888888888  \n");
	puts("    888888888888888888888888888888888888888888888888888888888888888888888  \n");

	puts("Build version: ");
	puts(OS_VERSION);
	puts("\n");

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

