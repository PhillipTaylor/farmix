#include <const.h>
#include <system.h>
#include "mem_struct.h"

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

void print_memory_map() {

	int i = 0;
	struct fm_mem_reserved *item;

	kprintf("Farmix Memory map:\n");

	for (i = 0; i < MEM_ARRAY_SIZE; i++) {

		if (fm_top_level_memory[i].active == 1) {
			kprintf("BLOCK %i: (%x%x -> %x%x)\n",
				i,
				(unsigned long)fm_top_level_memory[i].memory_start >> 32,
				(unsigned long)fm_top_level_memory[i].memory_start,
				(unsigned long)fm_top_level_memory[i].memory_end >> 32,
				(unsigned long)fm_top_level_memory[i].memory_end
			);
	
			item = fm_top_level_memory[i].head;

			if (item == NULL)
				kprintf("-- [entire block is free]\n");
			else {
				while (item != NULL) {
					kprintf("PROCESS %i HAS RANGE %x%x -> %x%x\n",
						item->owner_id,
						(unsigned long)item->memory_start >> 32,
						(unsigned long)item->memory_start,
						(unsigned long)item->memory_end >> 32,
						(unsigned long)item->memory_end
					);

					item = item->next;
				}
			}
		}

	}

	kprintf("End of Map\n");

}

