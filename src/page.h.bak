#ifndef PAGE_H
#define PAGE_H

#include <stdint.h>
#include <stddef.h>

void init_pfa_list(uint32_t total_mem_bytes, uint32_t kernel_end);
uint32_t allocate_physical_pages(size_t n);
void free_physical_pages(uint32_t paddr, size_t n);

#endif
