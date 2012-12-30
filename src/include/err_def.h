
#ifndef ERR_DEF_H
#define ERR_DEF_H

/* thrown by mem_api if a malloc request cannot be fulfilled. */
#define E_OUT_OF_MEMORY ((void*)-1)

/* thrown by ramdisk if you attempt to read past the end of its allocation */
#define E_DISK_BOUNDS -1

#endif
