#include <system.h>
#include <const.h>

#include <driver.h>

#ifndef OS_VERSION
	#define OS_VERSION "unspecified"
#endif

void ramdisk_testing();

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
	int *mem_req;

	mem_req = NULL;

	gdt_install();
	idt_install();
	isrs_install();
	irq_install();
	init_video();
	init_memory(grub1, magic);

	//testing malloc
	mem_req = (int*) malloc(sizeof(int));
	//mem_req = OUT_OF_MEMORY;

	if (mem_req == OUT_OF_MEMORY)
		kprintf("didn't get a usable address back\n");
	else {
		*mem_req = 6;
		kprintf("mem_req points to %x\n", mem_req);
		kprintf("mem_req has value of %i\n", *mem_req);
	}

	mem_req = (int*) malloc(16);
	//mem_req = (int*) malloc(99);
	//mem_req = (int*) malloc(1024);
	//free(mem_req);
	//mem_req = (int*) malloc(18024);
	//mem_req = (int*) malloc(1);

	// reprint memory map
	print_memory_map();
	print_welcome();
	//timer_install();
	keyboard_install();

	ramdisk_testing();

	__asm__ __volatile__ ("sti");

	for (;;);
}

void ramdisk_testing() {

	struct drv_device driv;
	char tx[20];
	char output[20];
	char *t = &tx[0];
	char *d = &output[0];
	int bc;

	t = ">> my data <<";
	ramdisk_install(&driv);
	print_memory_map();

	kprintf("ram disk size >> %x\n", driv.size);
	kprintf("ram disk files open >> %x\n", driv.files_open);

	driv.fp_open(&driv, "/asd/dsa.pdf", 'r');

	kprintf("ram disk files open >> %x\n", driv.files_open);

	//write some data!
	kprintf("writing to ramdisk:");
	puts(t);
	bc = driv.fp_write(&driv, 0xA314, t, 20);
	kprintf("bytes copied: %i\n", bc);

	bc = driv.fp_read(&driv, 0xA314, d, 20);
	kprintf("bytes copied: %i\n", bc);

	kprintf("output: ");
	puts(t);

}

