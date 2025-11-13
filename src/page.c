#include "page.h"

/* Assignment: 128 physical pages, each 2 MiB, covering 256 MiB total. */
#define NPHYS_PAGES 128u
#define FRAME_SIZE  (2u * 1024u * 1024u) /* 2 MiB */

static struct ppage physical_page_array[NPHYS_PAGES];

/* Head of the doubly-linked free list of pages */
static struct ppage *free_head = 0;

/* ---- minimal list helpers (internal) ---- */
static inline void list_push_front(struct ppage **head, struct ppage *node) {
    if (!node) return;
    node->prev = 0;
    node->next = *head;
    if (*head) (*head)->prev = node;
    *head = node;
}
static inline struct ppage* list_pop_front(struct ppage **head) {
    struct ppage *n = *head;
    if (!n) return 0;
    *head = n->next;
    if (*head) (*head)->prev = 0;
    n->next = n->prev = 0;
    return n;
}
static inline struct ppage* list_concat_front(struct ppage **head, struct ppage *lst) {
    /* splice lst (which may be a multi-node list) to the front of *head */
    if (!lst) return *head;
    /* find tail of lst */
    struct ppage *tail = lst;
    while (tail->next) tail = tail->next;
    /* stitch: lst + head */
    if (*head) {
        (*head)->prev = tail;
        tail->next = *head;
    }
    *head = lst;
    return *head;
}

/* PDF: init_pfa_list(void) — link every element into the free list and set physical_addr */
void init_pfa_list(void) {
    free_head = 0;
    for (unsigned i = 0; i < NPHYS_PAGES; ++i) {
        struct ppage *p = &physical_page_array[i];
        p->next = p->prev = 0;
        p->physical_addr = (void*)(i * FRAME_SIZE);
        list_push_front(&free_head, p);
    }
}

/* PDF: allocate_physical_pages(npages) — unlink npages from free list and return a new list */
struct ppage* allocate_physical_pages(unsigned int npages) {
    if (npages == 0) return 0;

    /* Ensure enough pages exist; if not, do nothing and return 0 */
    unsigned count = 0;
    for (struct ppage *cur = free_head; cur && count < npages; cur = cur->next) count++;
    if (count < npages) return 0;

    /* Pop npages from free_head, chaining them into allocd list (front insertion) */
    struct ppage *allocd = 0;
    for (unsigned i = 0; i < npages; ++i) {
        struct ppage *n = list_pop_front(&free_head);
        /* insert n at front of allocd */
        n->next = allocd;
        if (allocd) allocd->prev = n;
        allocd = n;
        n->prev = 0;
    }
    return allocd;
}

/* PDF: free_physical_pages(list) — return the list to the free list */
void free_physical_pages(struct ppage *ppage_list) {
    if (!ppage_list) return;
    /* make sure list prev is clean */
    ppage_list->prev = 0;
    /* concatenate to free_head */
    list_concat_front(&free_head, ppage_list);
}

/* ===== A4 paging bits (i386 4KiB pages) ===== */
#include <stdint.h>

/* Global, page-aligned directory and one page table (covers first 4 MiB). */
struct page_directory_entry g_pd[1024] __attribute__((aligned(4096)));
struct page                 g_pt0[1024] __attribute__((aligned(4096)));

/* Map a linked list of physical pages starting at vaddr.
   For A4 we only need PDE index 0 (addresses < 4 MiB). */
void *map_pages(void *vaddr, struct ppage *pglist, struct page_directory_entry *pd) {
    uintptr_t va = (uintptr_t)vaddr;
    struct ppage *p = pglist;

    while (p) {
        unsigned pde = (va >> 22) & 0x3FF;   /* top 10 bits */
        unsigned pti = (va >> 12) & 0x3FF;   /* next 10 bits */

        /* Ensure PDE present and points to our first page table (4KiB pages). */
        if (!pd[pde].present) {
            /* For this assignment we expect pde == 0 (first 4 MiB). */
            pd[pde].present = 1;
            pd[pde].rw      = 1;
            pd[pde].user    = 0;
            pd[pde].pagesize= 0; /* 0 => 4KiB pages */
            pd[pde].frame   = ((uint32_t)(uintptr_t)g_pt0) >> 12;
        }

        /* Grab the page table (only g_pt0 for this assignment). */
        struct page *pt = g_pt0;

        /* Fill the PT entry for this VA with the physical frame from pglist. */
        pt[pti].present = 1;
        pt[pti].rw      = 1;
        pt[pti].user    = 0;
        pt[pti].frame   = ((uint32_t)(uintptr_t)p->physical_addr) >> 12;

        /* Advance one page and one node. */
        va += 4096u;
        p = p->next;
    }

    return vaddr;
}
