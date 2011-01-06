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

 struct fm_mem_reserved  //reserved by programs calling malloc.
 {
 	int owner_id;
 	unsigned long long memory_start;
	unsigned long long memory_end;
	struct fm_mem_reserved *next;
 };
 
 struct fm_mem_block //the elements of static array. denote start/end and linked list of apps.
 {
 	int active; //0 = inactive, 1 = active
 	unsigned long long memory_start;
	unsigned long long memory_end;
	struct fm_mem_reserved *head;
 };


/*
 * This is the memory initialisation function. From grub we get two values:
 * 1) the value 0xBADBOO2 (which is magic number we use to _ensure_ we have "2)")
 * 2) a pointer to a data structure called multiboot_info which is basically a linked
 * list of available memory locations.
 *
 * So during init_memory we do the following, We loop over the structure and for
 * any free memory locations we create another data structure we use later during malloc
 * to hand out memory chunks. */

#define MEM_ARRAY_SIZE 15

struct fm_mem_block fm_top_level_memory[MEM_ARRAY_SIZE];

void init_memory(void *grub1, unsigned int magic)
{
	int i = 0;
	int offset = 0;

	struct multiboot_info *mbt;
	mbt = (struct multiboot_info*) grub1;
	
	kprintf("Initialising memory...\n");

	for (i = 0; i < MEM_ARRAY_SIZE; i++)
		fm_top_level_memory[i].active = 0; //initialise array

	if (magic == GRUB_MAGIC_NUMBER)
		kprintf("magic number match. %x\n", magic);
	else
		kprintf("magic number fail. expected: %x got: %x\n", GRUB_MAGIC_NUMBER, magic);

	i = 0;

	grub_multiboot_memory_map_t* mmap = (grub_multiboot_memory_map_t*) mbt->mmap_addr;
	while(mmap < mbt->mmap_addr + mbt->mmap_length) {

		kprintf("Memory Block #%i: %x%x length: %x%x (",
			i,
			mmap->base_addr_low,
			mmap->base_addr_high,
			mmap->length_low,
			mmap->length_high
		);

		if (mmap->type == FREE_MEMORY)
			kprintf("free memory)\n");
		else if (mmap->type == RESERVED)
			kprintf("reserved)\n");
		else if (mmap->type == ACPI_RECLAIMABLE)
		{
			kprintf("acpi reclaimable)\n");
			/* yeah I want to reclaim it, so just mark it as free */
			mmap->type = FREE_MEMORY;
		}
		else if (mmap->type == ACPI_NON_VOLATILE)
			kprintf("acpi non volatile)\n");
		else if (mmap->type == BAD)
			kprintf("bad)\n");

		if (mmap->type == FREE_MEMORY) {
			//copy it into a static arrray of memory blocks

			kprintf("Adding block to static array\n");

			//fm_top_level_memory[offset].active = 1;
			//fm_top_level_memory[offset].memory_start = mmap->base_addr_low;
			//fm_top_level_memory[offset].memory_end = fm_top_level_memory[i].memory_start + mmap->length;
			//fm_top_level_memory[offset].head = NULL; //completely empty!

			offset++;
		}

		i++;
		mmap = (grub_multiboot_memory_map_t*) ( (unsigned int)mmap + mmap->size + sizeof(unsigned int) );
	}

}

void print_memory_map() {

	int i = 0;
	struct fm_mem_reserved *item;
	void *x = NULL;


	kprintf("Farmix Memory map:\n");

	for (i = 0; i < MEM_ARRAY_SIZE; i++) {

		if (fm_top_level_memory[i].active == 1) {
			kprintf("BLOCK %i: (%X -> %X)\n",
				i,
				fm_top_level_memory[i].memory_start,
				fm_top_level_memory[i].memory_end
			);
	
			item = fm_top_level_memory[i].head;

			if (item == NULL)
				kprintf("-- [entire block is free]\n");
			else {
				while (item != NULL) {
					kprintf("PROCESS %i HAS RANGE %X -> %X\n",
						item->owner_id,
						item->memory_start,
						item->memory_end
					);

					item = item->next;
				}
			}
		}

	}

	kprintf("End of Map\n");

}

void *malloc(unsigned int num_bytes) {

	int i = 0, owner_id=39; //random owner_id...to be completed later.
	struct fm_mem_reserved *ptr;
	unsigned int space_required;
	unsigned int free_left = 0;
	struct fm_mem_reserved *item;

	//the link list describing memory actually comes from the the
	//unassigned memory blocks itself!!!
	space_required = num_bytes + sizeof(struct fm_mem_reserved);

	for (i = 0; i < MEM_ARRAY_SIZE; i++) {

		if (fm_top_level_memory[i].active == 1) {
	
			item = fm_top_level_memory[i].head;

			if (item == NULL) {
				// whole block free. is the whole block big enough?
				kprintf("first block is free. start: %X, end: %X\n",
					fm_top_level_memory[i].memory_start,
					fm_top_level_memory[i].memory_end
				);
				free_left = fm_top_level_memory[i].memory_end - fm_top_level_memory[i].memory_start;
				kprintf("free in this block: %i. required: %i. overhead: %i\n",
					free_left, space_required, sizeof(struct fm_mem_reserved));
				if (space_required < free_left) {

					//look down. look up. the address at memory start is now a pointer
					//to a struct fm_mem_reserved. I'm on a horse.
					ptr = (struct fm_mem_reserved*) fm_top_level_memory[i].memory_start;
					ptr->memory_start = (fm_top_level_memory[i].memory_start + sizeof(struct fm_mem_reserved));
					ptr->memory_end = (fm_top_level_memory[i].memory_start + space_required);
					ptr->next = NULL;
					ptr->owner_id = owner_id;

					fm_top_level_memory[i].head = ptr;

					return ptr->memory_start;
				}
			} else {
				kprintf("scanning for end of list\n");
				while (item->next != NULL)
					item = item->next;

				free_left = fm_top_level_memory[i].memory_end - item->memory_end;

				if (space_required < free_left) {
					;;
					//allocate this out!!
					//append data to structure and return. (do not continue)
				}
			}
		}
		//kprintf("moving to next block...\n");
	}

	kprintf("no block big enough\n");
	return -1;

}

void free(void *start_address) {
}
