#include <stdint.h>

/* -------- Multiboot2 header (required) -------- */
#define MULTIBOOT2_HEADER_MAGIC 0xe85250d6
const unsigned int multiboot_header[]
__attribute__((section(".multiboot"))) =
{
    MULTIBOOT2_HEADER_MAGIC,              /* magic */
    0,                                    /* architecture = i386 */
    16,                                   /* header length */
    -(16 + MULTIBOOT2_HEADER_MAGIC),      /* checksum */
    0, 12                                 /* end tag (type=0, size=12) */
};

/* -------- Port I/O helper -------- */
uint8_t inb(uint16_t _port) {
    uint8_t rv;
    __asm__ __volatile__ ("inb %1, %0" : "=a"(rv) : "dN"(_port));
    return rv;
}

/* -------- Simple VGA text-mode terminal driver -------- */
#define VGA_W      80
#define VGA_H      25
#define VGA_COLOR  0x07  /* light gray on black */
static volatile uint16_t *const VGA = (uint16_t*)0xB8000;
static int cur_row = 0, cur_col = 0;

static inline void vga_put_at(char ch, int r, int c) {
    VGA[r * VGA_W + c] = ((uint16_t)VGA_COLOR << 8) | (uint8_t)ch;
}

static void scroll_if_needed(void) {
    if (cur_row < VGA_H) return;
    /* scroll up by one row */
    for (int r = 1; r < VGA_H; ++r) {
        for (int c = 0; c < VGA_W; ++c) {
            VGA[(r - 1) * VGA_W + c] = VGA[r * VGA_W + c];
        }
    }
    /* clear last row */
    for (int c = 0; c < VGA_W; ++c) {
        VGA[(VGA_H - 1) * VGA_W + c] = ((uint16_t)VGA_COLOR << 8) | ' ';
    }
    cur_row = VGA_H - 1;
}

static void clear_screen(void) {
    for (int r = 0; r < VGA_H; ++r) {
        for (int c = 0; c < VGA_W; ++c) {
            VGA[r * VGA_W + c] = ((uint16_t)VGA_COLOR << 8) | ' ';
        }
    }
    cur_row = 0;
    cur_col = 0;
}

void putc(int data) {
    char ch = (char)data;

    if (ch == '\r') { cur_col = 0; return; }
    if (ch == '\n') { cur_col = 0; cur_row++; scroll_if_needed(); return; }
    if (ch == '\t') { int n = 4 - (cur_col & 3); while (n--) putc(' '); return; }

    vga_put_at(ch, cur_row, cur_col);
    if (++cur_col >= VGA_W) { cur_col = 0; cur_row++; }
    scroll_if_needed();
}

static void puts(const char *s) { while (*s) putc(*s++); }

static void print_uint(unsigned int x) {
    char buf[10]; int i = 0;
    if (x == 0) { putc('0'); return; }
    while (x && i < (int)sizeof(buf)) { buf[i++] = '0' + (x % 10); x /= 10; }
    while (i--) putc(buf[i]);
}
/* -------- end terminal driver -------- */

/* -------- Kernel entry -------- */
void main(void) {
    clear_screen();

    /* compute current privilege level (CPL = CS & 0x3) */
    unsigned short cs;
    __asm__ __volatile__("mov %%cs, %0" : "=r"(cs));
    unsigned int cpl = cs & 0x3;

    puts("Hello from my kernel!\r\n");
    puts("Current execution level (CPL/ring): ");
    print_uint(cpl);
    puts("\r\n");

    /* demo scroll: print a few extra lines */
    for (int i = 0; i < 40; ++i) {
        puts("Line "); print_uint(i); puts(": scrolling test...\r\n");
    }

    /* idle forever */
    for (;;)
        __asm__ __volatile__("hlt");
}

	
