#include "page.h"
#include <stdint.h>
#include <stddef.h>

#define PAGE_SIZE 4096u

static uint32_t g_total_mem_bytes = 0;
static uint32_t g_kernel_end = 0;

static inline uint32_t align_up_page(uint32_t x) {
    uint32_t rem = x % PAGE_SIZE;
    return rem ? (x + (PAGE_SIZE - rem)) : x;
}

void init_pfa_list(uint32_t total_mem_bytes, uint32_t kernel_end) {
    g_total_mem_bytes = total_mem_bytes;
    g_kernel_end = align_up_page(kernel_end);
    (void)g_total_mem_bytes;
    (void)g_kernel_end;
}

uint32_t allocate_physical_pages(size_t n) {
    (void)n;
    return 0;
}

void free_physical_pages(uint32_t paddr, size_t n) {
    (void)paddr;
    (void)n;
}
