#ifndef PAGE_H
#define PAGE_H

#include <stdint.h>
#include <stddef.h>

struct ppage {
    struct ppage *next;
    struct ppage *prev;
    void *physical_addr;
};

/* PDF-required API */
void              init_pfa_list(void);
struct ppage*     allocate_physical_pages(unsigned int npages);
void              free_physical_pages(struct ppage *ppage_list);

// ===== A4: 32-bit paging structs (i386) =====
struct page_directory_entry {
   uint32_t present       : 1;   // Page present in memory
   uint32_t rw            : 1;   // Read-only if clear, R/W if set
   uint32_t user          : 1;   // Supervisor only if clear
   uint32_t writethru     : 1;   // Page-level write-through
   uint32_t cachedisabled : 1;   // Disable caching
   uint32_t accessed      : 1;   // Accessed
   uint32_t pagesize      : 1;   // 0 = 4KiB pages
   uint32_t ignored       : 2;
   uint32_t os_specific   : 3;
   uint32_t frame         : 20;  // Frame address >> 12
};

struct page {
   uint32_t present    : 1;      // Page present in memory
   uint32_t rw         : 1;      // Read-only if clear, R/W if set
   uint32_t user       : 1;      // Supervisor only if clear
   uint32_t accessed   : 1;      // Accessed
   uint32_t dirty      : 1;      // Dirty
   uint32_t unused     : 7;
   uint32_t frame      : 20;     // Frame address >> 12
};

// A4 function: map a list of physical pages at virtual address vaddr using pd
void *map_pages(void *vaddr, struct ppage *pglist, struct page_directory_entry *pd);
#endif
