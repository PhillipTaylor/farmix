#include <const.h>
#include <system.h>
#include <err_def.h>
#include "mem_struct.h"

struct fm_mem_block fm_top_level_memory[MEM_ARRAY_SIZE];

// returns a reservation node correctly set up.
static struct fm_mem_reserved *make_block(unsigned long memory_start, unsigned long memory_end) {
	
	struct fm_mem_reserved *ptr;

	//look down. look up. the address at memory start is now a pointer
	//to a struct fm_mem_reserved. I'm on a horse.
	ptr = (struct fm_mem_reserved*) memory_start;
	ptr->memory_start = memory_start + sizeof(struct fm_mem_reserved); //start of the memory given to the process.
	ptr->memory_end = memory_end;
	ptr->next = NULL;

	ptr->owner_id = 39; //once scheduler exists would be sched->get_current_pid() or something.

	return ptr;
}

void *malloc(unsigned int num_bytes) {

	int i = 0;
	struct fm_mem_reserved *ptr;
	unsigned int space_required;
	unsigned int free_left = 0;
	struct fm_mem_reserved *item;

	// The link list describing memory actually comes from the
	// unassigned memory blocks itself!!!
	space_required = num_bytes + sizeof(struct fm_mem_reserved);

	for (i = 0; i < MEM_ARRAY_SIZE; i++) {

		if (fm_top_level_memory[i].active == TRUE) {
	
			item = fm_top_level_memory[i].head;

			/*
			 * WHOLE BLOCK IS FREE. ALLOCATE FIRST CHILD AND ASSIGN TO HEAD
			*/ 

			if (item == NULL) {
				free_left = fm_top_level_memory[i].memory_end - fm_top_level_memory[i].memory_start;

				if (space_required < free_left) {

					ptr = make_block(
						fm_top_level_memory[i].memory_start,
						fm_top_level_memory[i].memory_start + space_required
					);

					fm_top_level_memory[i].head = ptr;
					return (void*) ptr->memory_start;
				}

			} else {

				kprintf("scanning for end of list\n");

				/*
				 * ROOM BEFORE FIRST CHILD OF HEAD
				 * (first block allocated out from this top level was removed)
				*/

				if ((item->memory_start - fm_top_level_memory[i].memory_start) > space_required) {

					ptr = make_block(
						fm_top_level_memory[i].memory_start,
						fm_top_level_memory[i].memory_start + space_required
					);

					ptr->next = item;
					fm_top_level_memory[i].head = ptr;
					return (void*) ptr->memory_start;

				}

				/*
				 * ROOM BETWEEN TWO NODES
				 * (allocated block from the middle of the list was removed)
				*/

				while (item->next != NULL) {

					if ((item->next->memory_start - item->memory_end) > space_required) {
						//gap inside the linked list. hand it out.

						ptr = make_block(
							item->memory_end,
							item->memory_end + space_required
						);

						ptr->next = item->next;
						item->next = ptr;
						return (void*) ptr->memory_start;
					}

					item = item->next;
				}

				/*
				 * THE BLOCK ISN'T FULL AND SPACE IS AVAILABLE AT END OF LIST
				*/
				free_left = fm_top_level_memory[i].memory_end - item->memory_end;

				if (space_required < free_left) {
				
					ptr = make_block(
						item->memory_end,
						item->memory_end + space_required
					);

					item->next = ptr;
					return (void*) ptr->memory_start;
				}
			}
		}
	}

	kprintf("no block big enough\n");
	return E_OUT_OF_MEMORY;

}

void free(void *start_address) {

	int i = 0, srt;
	struct fm_mem_reserved *item;

	srt = (unsigned long) start_address;

	for (i = 0; i < MEM_ARRAY_SIZE; i++) {

		if (srt < fm_top_level_memory[i].memory_start) {
			kprintf("something went wrong. start address. less than first"
			"memory_start (start_address=%x, memory_start=%x\n",
				srt,
				fm_top_level_memory[i].memory_start
			);
		//It's in this top level block. Find it, delete it.
		} else if (srt < fm_top_level_memory[i].memory_end) {

			item = fm_top_level_memory[i].head;

			kprintf("read: %x\n", item->next->memory_start);

			if (item->memory_start == srt)
				fm_top_level_memory[i].head = item->next;

			while (item->next != NULL) {

				kprintf("read: %x\n",
					item->next->memory_start
				);

				if (item->next->memory_start == srt) {
					item->next = item->next->next;
					kprintf("freed.\n");
					return;
				}

				item = item->next;
			}

			kprintf("something went wrong. start_address doesn't match node\n");
			return;
		}
	}
	kprintf("something went wrong. start address greater than last"
	"memory_end (start_address=%x, memory_end=%x\n",
		srt,
		fm_top_level_memory[i].memory_end
	);
}

void print_memory_map() {

	int i = 0;
	struct fm_mem_reserved *item;

	kprintf("Farmix Memory map:\n");

	for (i = 0; i < MEM_ARRAY_SIZE; i++) {

		if (fm_top_level_memory[i].active == TRUE) {
			kprintf("BLOCK %i: (%x -> %x)\n",
				i,
				fm_top_level_memory[i].memory_start,
				fm_top_level_memory[i].memory_end
			);
	
			item = fm_top_level_memory[i].head;

			if (item == NULL)
				kprintf("-- [entire block is free]\n");
			else {
				while (item != NULL) {
					kprintf("PROCESS %i HAS RANGE %x -> %x\n",
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

