
/*
* Farmix uses linked lists to manage memory allocation. It's
* a very simplistic model to follow. Basically everytime
* malloc() is invoked we scan to the end of the list and
* add a new entry.
*
* It's not quite that simple however. The RAM we can allocate
* out isn't contigous. It normally comes in several segments.
* So farmix has a fixed size array of 15 elements, each
* representing the whole blocks available and each with a linked
* list chaining off of it, for example:
*
* [ FM_MEM_BLOCK 1, FM_MEM_BLOCK2, FM_MEM_BLOCK3 ]
*     (begin -> end)
*      |
*      |
*      FM_MEM_RESERVED
*        (3 bytes, process 3)
*      |
*      |
*      FM_MEM_RESERVED
*      	(7 bytes, process 3)
*/


struct fm_mem_reserved {
	int owner_id;
	unsigned long memory_start;
	unsigned long memory_end;
	struct fm_mem_reserved *next;
};

struct fm_mem_block {
	BOOL active; //0 = inactive, 1 = active
	unsigned long memory_start;
	unsigned long memory_end;
	struct fm_mem_reserved *head;
};

#define MEM_ARRAY_SIZE 15

extern struct fm_mem_block fm_top_level_memory[];
