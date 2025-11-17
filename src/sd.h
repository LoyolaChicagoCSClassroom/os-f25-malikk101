#ifndef __SD_H__
#define __SD_H__

int sd_readblock(unsigned int lba, void *buffer, unsigned int numsectors);

#endif
