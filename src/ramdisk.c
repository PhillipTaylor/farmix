
/*
 *  Farmix RAMDISK implementation
 *
 *  A Ramdisk, the simplest disk driver implementation.
*/

#include <system.h>
#include <driver.h>
#include <err_def.h>

int open(struct drv_device *dev, void *target, char mode);
int close(struct drv_device *dev, void *target);
size_t read(struct drv_device *dev, void *target, void *buffer, size_t bytes);
size_t write(struct drv_device *dev, void *target, void *buffer, size_t bytes);


// in future drivers will be loaded dynamically and
// need consistent entry points.
void ramdisk_install(struct drv_device *dev) {
	
	//hardcoded obviously until the whole filesystem stack is written.

	//dev->name = "RAMDISK";
	dev->size = 0xBFFF; //ramdisk
	dev->base_addr = malloc(dev->size);
	dev->fp_open = open;
	dev->fp_close = close;
	dev->fp_read = read;
	dev->fp_write = write;
	dev->files_open = 0;

	kprintf("base_addr: %x\n", dev->base_addr);
}

int open(struct drv_device *dev, void *target, char mode) {
	dev->files_open++;
	return 0;
}

int close(struct drv_device *dev, void *target) {
	dev->files_open--;
	return 0;
}

// only read as much as requested
size_t read(struct drv_device *dev, void *target, void *buffer, size_t bytes) {

	char *src = (char *)dev->base_addr + (size_t)target;
	char *max = (char *)dev->base_addr + (size_t)dev->size;

	if (src > max)
		return E_DISK_BOUNDS ; //don't copy, bad read command.
	
	memcpy(buffer, src, bytes);
	return bytes;

}

size_t write(struct drv_device *dev, void *target, void *buffer, size_t bytes) {

	char *src = (char *)dev->base_addr + (size_t)target;
	char *max = (char *)dev->base_addr + (size_t)dev->size;

	if (src > max)
		return E_DISK_BOUNDS ; //don't copy, bad write command.
	
	memcpy(src, buffer, bytes);
	return bytes;

}

