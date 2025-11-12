#include "page.h"
#include <stdint.h>
#include <stddef.h>

#define PAGE_SIZE 4096u
#define FREE_STACK_CAP 4096u  /* up to 4096 single pages cached */

static uint32_t g_total_mem_bytes = 0;
static uint32_t g_kernel_end = 0;
static uint32_t g_next_free = 0;

/* LIFO stack of freed single pages */
static uint32_t free_stack[FREE_STACK_CAP];
static size_t   free_top = 0;

static inline uint32_t align_up_page(uint32_t x) {
    uint32_t rem = x % PAGE_SIZE;
    return rem ? (x + (PAGE_SIZE - rem)) : x;
}
static inline uint32_t align_down_page(uint32_t x) {
    return x - (x % PAGE_SIZE);
}

void init_pfa_list(uint32_t total_mem_bytes, uint32_t kernel_end) {
    g_total_mem_bytes = align_down_page(total_mem_bytes);
    g_kernel_end      = align_up_page(kernel_end);
    g_next_free       = g_kernel_end;
    free_top          = 0;
}

/* For n==1: try pop from free stack; else bump-allocate contiguous n pages */
uint32_t allocate_physical_pages(size_t n) {
    if (n == 0) return 0;
    if (n == 1 && free_top > 0) {
        return free_stack[--free_top];
    }
    uint32_t size = (uint32_t)n * PAGE_SIZE;
    if (g_next_free + size > g_total_mem_bytes) return 0; /* OOM */
    uint32_t ret = g_next_free;
    g_next_free += size;
    return ret;
}

/* For n==1: push on free stack (best-effort). Multi-page frees are ignored for now. */
void free_physical_pages(uint32_t paddr, size_t n) {
    if (n == 1 && free_top < FREE_STACK_CAP) {
        free_stack[free_top++] = paddr;
        return;
    }
    (void)paddr; (void)n; /* ignore larger runs in this minimal version */
}
