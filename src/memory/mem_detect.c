#include <const.h>
#include <system.h>
#include "mem_struct.h"

/*
 * If the kernel is loaded by a multiboot compliant boot loader (such as grub)
 * It will probe the BIOS to determine the available and reserved memory and
 * then put a pointer to that structure in the ebx register. It will put the
 * value 0x2BADB002 is the eax register. If we find it, we know we have a
 * multiboot structure for our memory. If we don't shit outselves....because
 * we only support booting from grub!
 *
 * A typical physical memory map might look like this
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
 *
 * http://files.osdev.org/mirrors/geezer/osd/ram/index.htm#layout
*/

#define MULTIBOOT_MAGIC_NUMBER 0x2BADB002

#define FREE_MEMORY       1
#define RESERVED          2
#define ACPI_RECLAIMABLE  3 //means we can wipe it if we don't care about what's there.
#define ACPI_NON_VOLATILE 4
#define BAD               5

typedef struct multiboot_memory_map {
	unsigned int size;
	unsigned long base_addr_low;
	unsigned long base_addr_high;
	unsigned long length_low;
	unsigned long length_high;
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

/*
 * We basically loop over what we got from the multiboot
 * compliant bootloader's structure, defined above, take
 * the segments which are available and copy them to the
 * array fm_top_level_memory [mem_struct.h]. Then malloc
 * merely chains linked list nodes off them to denote
 * them as used.
*/

void init_memory(void *multiboot_data, unsigned int magic)
{
	int i = 0;
	int offset = 0;
	unsigned long total_memory;
	total_memory = 0;

	struct multiboot_info *mbt;
	mbt = (struct multiboot_info*) multiboot_data;
	
	kprintf("Initialising memory...\n");

	for (i = 0; i < MEM_ARRAY_SIZE; i++)
		fm_top_level_memory[i].active = FALSE; //initialise array

	if (magic == MULTIBOOT_MAGIC_NUMBER)
		kprintf("magic number match. %x\n", magic);
	else {
		kprintf("magic number fail. expected: %x got: %x\n", MULTIBOOT_MAGIC_NUMBER, magic);
		kprintf("memory management is unavailable. Did you boot from grub?\n");
		return;
	}

	i = 0;

	grub_multiboot_memory_map_t* mmap = (grub_multiboot_memory_map_t*) mbt->mmap_addr;
	while(mmap < mbt->mmap_addr + mbt->mmap_length) {

		kprintf("Memory Block #%i: %x length: %x (",
			i,
			mmap->base_addr_low,
			mmap->length_low
		);
		
		if (mmap->base_addr_high > 0 || mmap->length_high > 0) {
			kprintf("Woah! 64 bit address on a 32 bit operating system?\n");
			return;
		}

		if (mmap->type == FREE_MEMORY)
			kprintf("free memory)");
		else if (mmap->type == RESERVED)
			kprintf("reserved)");
		else if (mmap->type == ACPI_RECLAIMABLE)
		{
			kprintf("acpi reclaimable)");
			/* yeah I want to reclaim it, so just mark it as free */
			mmap->type = FREE_MEMORY;
		}
		else if (mmap->type == ACPI_NON_VOLATILE)
			kprintf("acpi non volatile)");
		else if (mmap->type == BAD)
			kprintf("bad)");

		if (mmap->type == FREE_MEMORY) {
			//copy it into a static arrray of memory blocks
			kprintf(" usable\n");

			fm_top_level_memory[offset].active = TRUE;

			if (mmap->base_addr_low == 0) {
				// address 0x0 in memory may be usable but you can't
				// _really_ use it because if I try and create a pointer
				// to that address the system confuses it with NULL,
				// so just throw base_addr 0 away because it's fucking useless.
				fm_top_level_memory[offset].memory_start = mmap->base_addr_low + 1;
				fm_top_level_memory[offset].memory_end = mmap->base_addr_low + mmap->length_low - 1;
				fm_top_level_memory[offset].head = NULL; //completely empty!

			} else {

				fm_top_level_memory[offset].memory_start = mmap->base_addr_low;
				fm_top_level_memory[offset].memory_end = mmap->base_addr_low + mmap->length_low;
				fm_top_level_memory[offset].head = NULL; //completely empty!
			}

			total_memory += mmap->length_low;
			offset++;
		} else
			kprintf(" ignored\n");

		i++;
		mmap = (grub_multiboot_memory_map_t*) ( (unsigned int)mmap + mmap->size + sizeof(unsigned int) );

	}
		
	kprintf("total usable memory: %x\n",
		total_memory
	);
}
