
#ifndef DRIVER_H
#define DRIVER_H

#include <types.h>

#define DRV_DEVICE_MAX_NAME 255

/*
 * Target device deals with literal addresses.
 * Represents a block device (or character device)
*/

struct drv_device {
	char name[DRV_DEVICE_MAX_NAME];
	int (*fp_open)(struct drv_device *dev, void *target, char mode);
	int (*fp_close)(struct drv_device *dev, void *target);
	size_t (*fp_read)(struct drv_device *dev, void *target, void* buffer, size_t bytes);
	size_t (*fp_write)(struct drv_device *dev, void *target, void *buffer, size_t bytes);

	void* base_addr;
	size_t size;
	size_t files_open;
};

/*
 * Filesystem driver.
*/

//struct FILE {
//	char *filename;
//	int state; // { OPEN, CLOSED }
//	int lock_pid; // lock_pid
//}


//struct drv_fs {
//	char *name; //ext3, fat, etc
//
//	FILE (*fp_open)(char *filename, char mode); //filename relational from mountpoint
//	size_t (*fp_close)(void);
//	size_t (*fp_read)(FILE fh, void *target, size_t bytes);
//	size_t (*fp_write)(FILE fh, void *source, size_t bytes);
//	size_t (*fp_get_size)();
//}

/* 
 * VFS Entry!
 *
 * This is most easily visualised as a record in /etc/fstab
 *
 * It's the the data structure for the vfs (Virtual File System)
 * It ties mount points to filesystem drivers to block device drivers.
*/

//struct vfs_entry {
//	char *mountpoint;
//	char *drv_fs;
//	struct *dvr_device;
//}

#endif
