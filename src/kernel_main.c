#include <stdint.h>
#include "interrupt.h"
#include "page.h"
#include "fat.h"
#include "sd.h"

/* -------- Multiboot2 header (required) -------- */
#include <stdint.h>
#include "interrupt.h"
#include "page.h"
#include "fat.h"
#include "sd.h"
#include "screen.h"

/* -------- Multiboot2 header (required) -------- */
#define MULTIBOOT2_HEADER_MAGIC 0xe85250d6
const unsigned int multiboot_header[]
__attribute__((section(".multiboot"))) =
{
    MULTIBOOT2_HEADER_MAGIC,
    0,                     /* i386 */
    16,                    /* header length */
    -(16 + MULTIBOOT2_HEADER_MAGIC),
    0, 12
};

/* -------- Port I/O helper -------- */
uint8_t inb(uint16_t _port) {
    uint8_t rv;
    __asm__ __volatile__("inb %1, %0" : "=a"(rv) : "dN"(_port));
    return rv;
}

/* ------------------------------------------------------- */
/* ----------------------   MAIN   ------------------------ */
/* ------------------------------------------------------- */

void main() {

    puts("PFA init called.\r\n");

    /* Print privilege level */
    unsigned short cs;
    __asm__ __volatile__("mov %%cs, %0" : "=r"(cs));
    unsigned int cpl = cs & 0x3;

    puts("Hello from my kernel!\r\n");
    puts("Current execution level (CPL/ring): ");
    print_uint(cpl);
    puts("\r\n");

    /* ------------------ Paging (Assignment 4) ------------------ */
    init_pfa_list();   /* your Assignment 4 function */

    /* Scroll test */
    for (int i = 0; i < 8; ++i) {
        puts("Line ");
        print_uint(i);
        puts(": scrolling test...\r\n");
    }

    /* ------------------ PFA Test ------------------ */
    struct ppage* a = allocate_physical_pages(1);
    puts("alloc 1 page -> ");
    print_uint((unsigned int)(a ? (unsigned int)a->physical_addr : 0));
    puts("\r\n");

    struct ppage* b = allocate_physical_pages(2);
    puts("alloc 2 pages -> ");
    print_uint((unsigned int)(b ? (unsigned int)b->physical_addr : 0));
    puts("\r\n");

    free_physical_pages(a);
    puts("freed 1 page starting at ");
    print_uint((unsigned int)(a ? (unsigned int)a->physical_addr : 0));
    puts("\r\n");

    struct ppage* c = allocate_physical_pages(1);
    puts("alloc again 1 page -> ");
    print_uint((unsigned int)(c ? (unsigned int)c->physical_addr : 0));
    puts("\r\n");

    puts("PFA post-loop check...\r\n");
    struct ppage* d = allocate_physical_pages(1);
    puts("post alloc 1 page -> ");
    print_uint((unsigned int)(d ? (unsigned int)d->physical_addr : 0));
    puts("\r\n");
    free_physical_pages(d);

    struct ppage* e = allocate_physical_pages(1);
    puts("post alloc again -> ");
    print_uint((unsigned int)(e ? (unsigned int)e->physical_addr : 0));
    puts("\r\n");

    /* ------------------ Assignment 5 ------------------ */

    unsigned char sec[512];

    puts("\r\nHW5 (late): reading MBR LBA0...\r\n");
    if (sd_readblock(0, sec, 1) == 0) {
        puts("MBR sig[510,511]=");
        print_uint(sec[510]); putc(',');
        print_uint(sec[511]);
        puts("\r\n");
    } else {
        puts("MBR read FAIL\r\n");
    }

    puts("HW5 (late): reading FAT boot sector LBA2048...\r\n");
    if (sd_readblock(2048, sec, 1) == 0) {
        unsigned short bps = sec[11] | (sec[12]<<8);
        unsigned char  spc = sec[13];
        unsigned short rs  = sec[14] | (sec[15]<<8);

        puts("BS: bps="); print_uint(bps);
        puts(" spc="); print_uint(spc);
        puts(" rs=");  print_uint(rs);
        puts("\r\n");
    } else {
        puts("BS read FAIL\r\n");
    }

    if (fatInit()==0) puts("FAT init OK\r\n");
    else puts("FAT init FAIL\r\n");

    struct file F;
    if (fatOpen("KERNEL", &F)==0) {
        puts("fatOpen KERNEL: size=");
        print_uint(F.rde.file_size);
        puts(" clus=");
        print_uint(F.start_cluster);
        puts("\r\n");

        unsigned char buf[512];
        int n = fatRead(&F, buf, 512);

        puts("fatRead first 32 bytes: ");
        for (int i=0; i < (n<32?n:32); i++) {
            print_uint(buf[i]);
            putc(' ');
        }
        puts("\r\n");
    } else {
        puts("fatOpen KERNEL: FAIL\r\n");
    }

    for (;;) {}
}
