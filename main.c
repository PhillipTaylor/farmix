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
	int l1, l2;

    puts("  Welcome to Farmix!\n");
    puts("   Operating System\n");
    puts("       ");
	puts(OS_VERSION);
	puts("\n");

	l1 = NULL;
	
	kprintf("NULL = %i\n", &l1, NULL, NULL, NULL, NULL);

	l1 = 15;
	l2 = 10;

	kprintf("testing the new and sexy kprintf: (%x = 15 and %x = 10)\n", &l1, &l2, NULL, NULL, NULL);

}

void _start(void *grub1, unsigned int magic)
{
    gdt_install();
    idt_install();
    isrs_install();
    irq_install();
    init_video();
	init_memory(grub1, magic);
	print_welcome();
    timer_install();
    keyboard_install();

    __asm__ __volatile__ ("sti");

    for (;;);
}

