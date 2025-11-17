#include "fat.h"
#include "sd.h"

static unsigned int g_bps, g_spc, g_rs, g_nf, g_spf, g_nroot;
static unsigned int g_first_fat_sector, g_root_dir_sector, g_root_dir_sectors, g_first_data_sector;

static unsigned int rd16(const unsigned char *p) {
    return (unsigned int)p[0] | ((unsigned int)p[1] << 8);
}

static void to_83(const char *name, char out[11]) {
    for (int i = 0; i < 11; i++) out[i] = ' ';
    int base = 0, ext = 8, in_ext = 0;
    for (const char *p = name; *p; ++p) {
        char c = *p;
        if (c == '.') { in_ext = 1; continue; }
        if (c >= 'a' && c <= 'z') c -= 32;
        if (!in_ext) { if (base < 8) out[base++] = c; }
        else { if (ext < 11) out[ext++] = c; }
    }
}

unsigned int fatFirstSectorOfCluster(unsigned int cluster) {
    if (cluster < 2) cluster = 2;
    return g_first_data_sector + (cluster - 2) * g_spc;
}

static unsigned int fatNextCluster(unsigned int cluster) {
    unsigned int fat_off = cluster * 2;
    unsigned int sector  = g_first_fat_sector + (fat_off / g_bps);
    unsigned int off     = fat_off % g_bps;
    unsigned char sec[SECTOR_SIZE];

    if (sd_readblock(sector, sec, 1) != 0) return 0;
    return (unsigned int)sec[off] | ((unsigned int)sec[off+1] << 8);
}

int fatInit(void) {
    unsigned char bs[SECTOR_SIZE];
    if (sd_readblock(2048, bs, 1) != 0) return -1;

    g_bps   = rd16(&bs[11]);
    g_spc   = bs[13];
    g_rs    = rd16(&bs[14]);
    g_nf    = bs[16];
    g_nroot = rd16(&bs[17]);
    g_spf   = rd16(&bs[22]);

    g_first_fat_sector  = 2048 + g_rs;
    g_root_dir_sector   = g_first_fat_sector + g_nf * g_spf;
    g_root_dir_sectors  = ((g_nroot * 32) + (g_bps - 1)) / g_bps;
    g_first_data_sector = g_root_dir_sector + g_root_dir_sectors;

    if (g_bps != SECTOR_SIZE) return -1;
    return 0;
}

int fatOpen(const char *name, struct file *out) {
    unsigned char target[11];
    to_83(name, (char*)target);

    unsigned char sec[SECTOR_SIZE];

    for (unsigned int s = 0; s < g_root_dir_sectors; s++) {
        if (sd_readblock(g_root_dir_sector + s, sec, 1) != 0) return -1;

        for (int i = 0; i < (SECTOR_SIZE / 32); i++) {
            unsigned char *e = &sec[i*32];
            if (e[0] == 0x00) return -1;
            if (e[0] == 0xE5) continue;
            if (e[11] == 0x0F) continue;

            int match = 1;
            for (int k = 0; k < 11; k++)
                if (e[k] != target[k]) { match = 0; break; }
            if (!match) continue;

            for (int k = 0; k < 8; k++) out->rde.file_name[k] = e[k];
            for (int k = 0; k < 3; k++) out->rde.file_extension[k] = e[8+k];

            out->rde.attribute = e[11];
            out->rde.file_size =
                  (unsigned int)e[28]
                | ((unsigned int)e[29] << 8)
                | ((unsigned int)e[30] << 16)
                | ((unsigned int)e[31] << 24);

            out->rde.cluster = rd16(&e[26]);

            out->start_cluster = out->rde.cluster;
    out->pos = 0;
    out->cur_cluster = out->start_cluster;
            out->pos = 0;
            out->cur_cluster = out->start_cluster;

            return 0;
        }
    }
    return -1;
}

int fatRead(struct file *f, void *buf, unsigned int nbytes) {
    if (!f) return -1;
    unsigned int filesize = f->rde.file_size;

    if (f->pos >= filesize) return 0;
    if (nbytes > filesize - f->pos)
        nbytes = filesize - f->pos;

    unsigned int cluster_size = g_spc * g_bps;

    unsigned int pos = f->pos;
    unsigned int cluster = f->start_cluster;

    unsigned int skip = pos / cluster_size;
    for (unsigned int i = 0; i < skip; i++) {
        cluster = fatNextCluster(cluster);
        if (cluster >= 0xFFF8 || cluster == 0) break;
    }

    unsigned int offset_in_cluster = pos % cluster_size;
    unsigned int sector_in_cluster = offset_in_cluster / g_bps;
    unsigned int offset_in_sector  = offset_in_cluster % g_bps;

    unsigned char sec[SECTOR_SIZE];
    unsigned char *dst = (unsigned char*)buf;
    unsigned int copied = 0;

    while (copied < nbytes) {
        unsigned int lba = fatFirstSectorOfCluster(cluster) + sector_in_cluster;
        if (sd_readblock(lba, sec, 1) != 0) break;

        unsigned int left_in_sector = g_bps - offset_in_sector;
        unsigned int left_need = nbytes - copied;
        unsigned int take = (left_in_sector < left_need) ? left_in_sector : left_need;

        for (unsigned int i = 0; i < take; i++)
            dst[copied + i] = sec[offset_in_sector + i];

        copied += take;
        offset_in_sector = 0;
        sector_in_cluster++;

        if (sector_in_cluster >= g_spc) {
            sector_in_cluster = 0;
            cluster = fatNextCluster(cluster);
            if (cluster >= 0xFFF8 || cluster == 0) break;
        }
    }

    f->pos += copied;
    f->cur_cluster = cluster;

    return copied;
}

