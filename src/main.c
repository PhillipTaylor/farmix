#include <system.h>
#include <const.h>

#include <driver.h>
#include <err_def.h>

#ifndef OS_VERSION
	#define OS_VERSION "unspecified"
#endif

void ramdisk_testing();
void disktesting();

unsigned char inb (unsigned short _port) {
	unsigned char rv;
	__asm__ __volatile__ ("inb %1, %0" : "=a" (rv) : "dN" (_port));
	return rv;
}

void outb (unsigned short _port, char _data) {
	__asm__ __volatile__ ("outb %1, %0" : : "dN" (_port), "a" (_data));
}

unsigned short inw (unsigned short _port) {
	unsigned short rv;
	__asm__ __volatile__ ("inw %1, %0" : "=a" (rv) : "dN" (_port));
	return rv;
}

void outw (unsigned short _port, unsigned int _data) {
	__asm__ __volatile__ ("outw %1, %0" : : "dN" (_port), "a" (_data));
}

void stall(int times) {
    // port 0x80 is used for 'checkpoints' during POST.
	// The Linux kernel seems to think it is free for use :-/
	do {
		asm volatile( "outb %%al, $0x80" : : "a"(0) );
	} while (times-- > 0);
}

void print_welcome() {

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

void _start(void *grub1, unsigned int magic) {
	int *mem_req;

	mem_req = NULL;

	gdt_install();
	idt_install();
	isrs_install();
	irq_install();
	init_video();
	init_memory(grub1, magic);
	timer_install();
	keyboard_install();
	disktesting();

	//works on eeepc, not on VBox
	//__asm__ __volatile__ ("sti");

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
	bc = driv.fp_write(&driv, (void*)0xA314, t, 20);
	kprintf("bytes copied: %i\n", bc);

	bc = driv.fp_read(&driv, (void*)0xA314, d, 20);
	kprintf("bytes copied: %i\n", bc);

	kprintf("output: ");
	puts(t);

}

void disktesting() {

	size_t lba = 1021956;
	char buffer[256];
	char *p = &buffer[0];
	int i;

	cls();
	puts("reading disk...\n");

	memset(p, 0, 256); //set to blank

	ata_pio_read(lba, p, 1); //read from disk

	//print it out in hex
	puts("ata_pio_read_finished. data as below:\n");
	for (i = 0; i < 13; i++) {
		kprintf("%x ", buffer[i]);
	}
	kprintf("\n");
	kprintf(&buffer[0]);

}

