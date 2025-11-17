#ifndef FAT_H
#define FAT_H

#include <stdint.h>

#define SECTOR_SIZE 512

struct root_dir_entry {
    unsigned char file_name[8];
    unsigned char file_extension[3];
    unsigned char attribute;
    unsigned char reserved[10];
    uint16_t      cluster;
    uint32_t      file_size;
} __attribute__((packed));

struct file {
    struct root_dir_entry rde;
    uint16_t start_cluster;
    uint32_t pos;
    uint32_t cur_cluster;

    /* stream fields */
};

int fatInit(void);
int fatOpen(const char *name, struct file *out);
int fatRead(struct file *f, void *buf, unsigned int nbytes);

unsigned int fatFirstSectorOfCluster(unsigned int cluster);

#endif
