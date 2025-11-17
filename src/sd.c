#include "ide.h"
#include "sd.h"

int sd_readblock(unsigned int lba, void *buffer, unsigned int numsectors) {
    // Forward to your IDE PIO LBA reader
    return ata_lba_read(lba, (unsigned char*)buffer, numsectors);
}
