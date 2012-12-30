
/*
 * Farmix implementations of an ATA disk driver, using shitty ATA PIO.
*/

#include <system.h>

#define ATA_PIO_UNINITIALISED   0
#define ATA_PIO_READY           1
#define ATA_PIO_BUSY            2
#define ATA_PIO_LOCKED          3

/* called 18 times per second by timer.c so we can poll the hard disk */
void ata_pio_poller();


void ata_pio_install() {

	timer_install_tick_handler(ata_pio_poller); // register our callback.
	
}

void ata_pio_poller() {
	//puts("our ata_pio driver is recieving callbacks ok.\n");
}

void ata_pio_read(int sector, int lba, void *buffer, size_t count) {
}

// todo
void ata_pio_write(int sector, int lba, void *buffer, size_t count) {
}
