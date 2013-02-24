
/*
 * Farmix implementations of an ATA disk driver, using shitty ATA PIO.
*/

#include <system.h>
#include <types.h>
#include <const.h>

BOOL primary_detected = FALSE;
unsigned short disk_info[256];
BOOL pio_48_supported = FALSE;
unsigned int sector_count = 0;


/* called 18 times per second by timer.c so we can poll the hard disk */
void ata_pio_poller();
void ata_pio_disk_reset();


void ata_pio_install() {
	char detect;
	unsigned short tmp;
	unsigned int i, tmp2;

	puts("ata_pio_install()\n");
	//ata_pio_disk_reset();
	puts("initialising disk\n");
	//timer_install_tick_handler(ata_pio_poller); // register our callback.
	//To use the IDENTIFY command, select a target drive by sending 0xA0 for the master drive, or 0xB0 for the slave, to the "drive select" IO port.
	//On the Primary bus, this would be port 0x1F6.
	outb(0x1F6, 0xA0);

	//Then set the Sectorcount, LBAlo, LBAmid, and LBAhi IO ports to 0 (port 0x1F2 to 0x1F5).
	outb(0x1F2, 0);
	outb(0x1F3, 0);
	outb(0x1F4, 0);
	outb(0x1F5, 0);

	//Then send the IDENTIFY command (0xEC) to the Command IO port (0x1F7).
	outb(0x1F7, 0xEC);

	//Then read the Status port (0x1F7) again. If the value read is 0, the drive does not exist.
	detect = inb(0x1F7);

	if (detect != 0) {
		primary_detected = TRUE;
		puts("primary drive detected\n");
	} else {
		puts("drive does not exist\n");
		return;
	}

	//For any other value: poll the Status port (0x1F7) until bit 7 (BSY, value = 0x80) clears.
	while ((inb(0x1F7) & 0x80) == 0x80) ; //wait until BUS BUSY bit to be cleared
	
	//Because of some ATAPI drives that do not follow spec, at this point you need to check the LBAmid and LBAhi ports (0x1F4 and 0x1F5)
	//to see if they are non-zero. If so, the drive is not ATA, and you should stop polling.

	detect = inb(0x1F4);
	detect += inb(0x1F5);

	if (detect > 0) {
		puts("primary disk detected is ATAPI (CD-ROM style) not PATA\n");
		return;
	}

	//Otherwise, continue polling one of the
	//Status ports until bit 3 (DRQ, value = 8) sets, or until bit 0 (ERR, value = 1) sets.
	//do {
	//   detect = inb(0x1F7);
	//} while (detect && 0x04);
	while ((inb(0x1F7) & 0x40) == 0x40) ; // wait for DRIVE READY bit to be set

	//At that point, if ERR is clear, the data is ready to read from the Data port (0x1F0). Read 256 words, and store them.
	for (i = 0; i < 256; i++) {
		disk_info[i] = inw(0x1F0);
	}

	//bit 10 of word 83 determines if the disk supports read48 mode.
	if ((disk_info[83] & 0x10) == 0x10) {
		pio_48_supported = TRUE;
		puts("pio 48 read supported\n");
	} else {
		puts("pio 48 read NOT supported\n");
		tmp2 = disk_info[83];
		kprintf("word 83 = %x\n", tmp2);
	}

	tmp2 = disk_info[60]; //word size
	tmp2 = tmp2 << 16;
	tmp2 += disk_info[61];
	kprintf("disk supports %x total lba addressable sectors in pio 28 mode\n");
	sector_count = tmp2;

	//ata_pio_disk_reset();

}

void ata_pio_poller() {
	//puts("our ata_pio driver is recieving callbacks ok.\n");
}

//void ata_pio_read(int sector, int lba, void *buffer, size_t count) {
//}

// todo
//void ata_pio_write(int sector, int lba, void *buffer, size_t count) {
//}

void reset() {
}

void ata_pio_disk_reset() {
 // There is an additional IO port that changes the behavior of each ATA bus, called the
 // Device Control Register (on the Primary bus, port 0x3F6)
 // For non-ATAPI drives, the only method a driver has of resetting a drive after a major error
 // is to do a "software reset" on the bus. Set bit 2 (SRST, value = 4) in the proper Control
 // Register for the bus. This will reset both ATA devices on the bus.
 //outb(0x3F6, 0x4);

 //Then, you have to clear that bit again, yourself.
 //outb(0x3F6, 0x0);
}

void ata_pio_read(size_t lba, void *buffer, size_t count) {
   size_t bytes_read;
   char *buff = (char*)buffer;
   unsigned short word;
   char chr;
   int tmp;
   //primary bus
   unsigned short port = 0x1F0;
   unsigned short slavebit = 0;

	//cls();
	kprintf("ata_pio_read: %x, %x, %i\n", lba, buff, count);

   if (count == 0) { //nothing to read.
        kprintf("nothing to read..\n");
        return;
   }

   if ((inb(port+7) & 0x08) != 0x08) {
		ata_pio_disk_reset();
		puts("disk reset required\n");
	}

   //wait until DRQ bit is set
   //while ((inb(port+7) & 0x08) != 0x08)
		/* no op */;

   //Send 0xE0 for the "master" or 0xF0 for the "slave", ORed with the highest 4 bits of the LBA to port 0x1F6:
   outb(0x1F6, 0xE0 | (slavebit << 4) | ((lba >> 24) & 0x0F));
   //outb(0x1F6, 0xE0 | ((lba >> 24) & 0x0F));

   //Send a NULL byte to port 0x1F1, if you like (it is ignored and wastes lots of CPU time):
   outb(0x1F1, 0x00);
   //stall(5);

   //Send the sectorcount to port 0x1F2:
   //outb(0x1F2, (unsigned char) sector_count);
   // THINK THIS IS THE AMOUNT OF DATA TO READ!! NOT THE VOLUME SIZE COLLECTED EARLIER!

   outb(0x1F2, 0x01);  //READ ONE SECTOR (expect 512 bytes!?)

   //Send the low 8 bits of the LBA to port 0x1F3:
   outb(0x1F3, (unsigned char) lba);

   //Send the next 8 bits of the LBA to port 0x1F4:
   outb(0x1F4, (unsigned char)(lba >> 8));

   //Send the next 8 bits of the LBA to port 0x1F5:
   outb(0x1F5, (unsigned char)(lba >> 16));

   //outb(0x1F6, 0xE0 | (1 << 4) | ((lba >> 24) & 0x0F));

   //Send the "READ SECTORS" command (0x20) to port 0x1F7:
   outb(0x1F7, 0x20);

	kprintf("got here >> : %x, %x, %i\n", lba, buff, count);
    //Wait for an IRQ or poll.
	for(tmp = 0; tmp < 3000; tmp++)
	{}

	puts("waiting on BUS BUSY to be cleared\n");
	while ((inb(0x1F7) & 0x80) != 0x0) {} //wait until BUS BUSY bit to be cleared
	puts("waiting for DRIVE READY to be set\n");
	while ((inb(0x1F7) & 0x40) != 0x40) {} // wait for DRIVE READY bit to be set

	kprintf("got here: %x, %x, %i\n", lba, buff, count);
	//Transfer 256 words, a word at a time, into your buffer from I/O port 0x1F0. (In assembler, REP INSW works well for this.)
	//(word = 2 bytes to 256 word = 512 bytes)

	bytes_read = 0;

	while (bytes_read < 256 ) {//&& count > 0) {

		word = 0;
		word = inw(0x1F0);

		buff[bytes_read * 2] = word & 0xFF;
		buff[(bytes_read * 2) + 1] = word >> 8;

		bytes_read++;
		//count--;
	}

	kprintf("%i bytes read!\n", bytes_read);

}

