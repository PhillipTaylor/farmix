#include <system.h>

void *memcpy(void *dest, const void *src, size_t count)
{
    const char *sp = (const char *)src;
    char *dp = (char *)dest;
    for(; count != 0; count--) *dp++ = *sp++;
    return dest;
}

void *memset(void *dest, char val, size_t count)
{
    char *temp = (char *)dest;
    for( ; count != 0; count--) *temp++ = val;
    return dest;
}

unsigned short *memsetw(unsigned short *dest, unsigned short val, size_t count)
{
    unsigned short *temp = (unsigned short *)dest;
    for( ; count != 0; count--) *temp++ = val;
    return dest;
}

/*
 * -----------------------------------------------------
 *  Detect Memory (by using GRUB's memory structure loaded for us.
 * -----------------------------------------------------
 * Physical Memory Map
 *
 * http://files.osdev.org/mirrors/geezer/osd/ram/index.htm#layout
 *
 * start    end      type   address
 * ---------------------------------------------------------
 *      0        3FF   RAM  Real Mode Interrupt Vector Table
 *    400        4FF   RAM  BDA (BIOS Data Area)
 *    500      9FBFF   RAM  Free, _actual_ usable memory area
 *  9FC00      9FFFF   RAM  EXBA (Extended BIOS Data Area)
 *  A0000      BFFFF Video  VGA Framebuffers
 *  C0000      C7FFF   ROM  Video BIOS
 *  C8000      EFFFF   ---- unknown, possibly usable ----
 *  F0000      FFFFF   ROM  Motherboard BIOS (64K is typical)
 * 100000   FEBFFFFF   RAM  Free Extended Memory - must probe to figure out what's usable
 * FEC00000 FFFFFFFF   ---  Motherboard BIOS, Plug 'n' Play, ACPI, Suspend Service Management etc.
 *
*/

/*
 * If the kernel is loaded by a multiboot compliant boot loader (such as grub)
 * It will probe the BIOS to determine the available and reserved memory and
 * then put a pointer to that structure in the ebx register. It will put the
 * value 0x2BADB002 is the eax register. If we find it, we know we have a
 * multiboot structure for our memory. If we don't shit outselves....because
 * we only support booting from grub!
*/
#define GRUB_MAGIC_NUMBER 0x2BADB002

/* You can't detect RAM yourself because you're already using it.
 * Most people ask the BIOS. This is the only reliable way. However
 * because we're booting from grub, it's done the hard work for us
 * and leaves us a multiboot_memory_map in the eax register (I know,
 * how sweet :-) ). My start.asm puts that pointer into this variable
 * so we can just take what grubs given us.
*/
unsigned int grub_boot_memory_map;

#define FREE_MEMORY       1
#define RESERVED          2
#define ACPI_RECLAIMABLE  3 //means we can wipe it if we don't care about what's there.
#define ACPI_NON_VOLATILE 4
#define BAD               5

typedef struct multiboot_memory_map {
	unsigned int size;
	unsigned int base_addr_low;
	unsigned int base_addr_high;
	unsigned int length_low;
	unsigned int length_high;
	unsigned int type;
} grub_multiboot_memory_map_t;

//see from multiboot.h (a part of grub).

 /* The symbol table for a.out. */
 typedef struct aout_symbol_table
 {
   unsigned long tabsize;
   unsigned long strsize;
   unsigned long addr;
   unsigned long reserved;
 } aout_symbol_table_t;
 
 /* The section header table for ELF. */
 typedef struct elf_section_header_table
 {
   unsigned long num;
   unsigned long size;
   unsigned long addr;
   unsigned long shndx;
 } elf_section_header_table_t;
 
 /* The Multiboot information. */
 struct multiboot_info
 {
   unsigned long flags;
   unsigned long mem_lower;
   unsigned long mem_upper;
   unsigned long boot_device;
   unsigned long cmdline;
   unsigned long mods_count;
   unsigned long mods_addr;
   union
   {
	 aout_symbol_table_t aout_sym;
	 elf_section_header_table_t elf_sec;
   } u;
   unsigned long mmap_length;
   unsigned long mmap_addr;
 };

void init_memory(void *grub1, unsigned int magic)
{
	int i = 0;
	struct multiboot_info *mbt;
	mbt = (struct multiboot_info*) grub1;
	
	puts("Initialising memory...\n");

	if (magic == GRUB_MAGIC_NUMBER)
		kprintf("magic number match. %i\n", magic);
	else
		kprintf("magic number fail. expected: %x got: %x\n", GRUB_MAGIC_NUMBER, magic);

	grub_multiboot_memory_map_t* mmap = (grub_multiboot_memory_map_t*) mbt->mmap_addr;
	while(mmap < mbt->mmap_addr + mbt->mmap_length) {

		kprintf("Memory Block #%i: %x -> %x (", &i++, &mmap->base_addr_low, &mmap->base_addr_high, NULL, NULL);

		if (mmap->type == FREE_MEMORY)
			kprintf("free memory)\n", NULL, NULL, NULL, NULL, NULL);
		else if (mmap->type == RESERVED)
			kprintf("reserved)\n", NULL, NULL, NULL, NULL, NULL);
		else if (mmap->type == ACPI_RECLAIMABLE)
		{
			kprintf("acpi reclaimable)\n", NULL, NULL, NULL, NULL, NULL);
			/* yeah I want to reclaim it, so just mark it as free */
			mmap->type = FREE_MEMORY;
		}
		else if (mmap->type == ACPI_NON_VOLATILE)
			kprintf("acpi non volatile)\n", NULL, NULL, NULL, NULL, NULL);
		else if (mmap->type == BAD)
			kprintf("bad)\n", NULL, NULL, NULL, NULL, NULL);

		mmap = (grub_multiboot_memory_map_t*) ( (unsigned int)mmap + mmap->size + sizeof(unsigned int) );
	}

}

