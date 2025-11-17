#include "screen.h"

static int cur_row = 0;
static int cur_col = 0;

static inline void vga_put_at(char ch, int r, int c) {
    VGA[r * VGA_W + c] = ((unsigned short)VGA_COLOR << 8) | (unsigned char)ch;
}

void scroll_if_needed(void) {
    if (cur_row < VGA_H) return;

    for (int r = 1; r < VGA_H; ++r)
        for (int c = 0; c < VGA_W; ++c)
            VGA[(r-1) * VGA_W + c] = VGA[r * VGA_W + c];

    for (int c = 0; c < VGA_W; ++c)
        VGA[(VGA_H-1) * VGA_W + c] = ((unsigned short)VGA_COLOR << 8) | ' ';

    cur_row = VGA_H - 1;
}

void putc(char ch) {
    if (ch == '\n' || ch == '\r') {
        cur_row++;
        cur_col = 0;
        scroll_if_needed();
        return;
    }
    vga_put_at(ch, cur_row, cur_col);
    cur_col++;

    if (cur_col >= VGA_W) {
        cur_col = 0;
        cur_row++;
        scroll_if_needed();
    }
}

void puts(const char *s) {
    while (*s) putc(*s++);
}

void print_uint(unsigned int x) {
    char buf[16];
    int i = 0;
    if (x == 0) { putc('0'); return; }
    while (x > 0) {
        buf[i++] = '0' + (x % 10);
        x /= 10;
    }
    while (i--) putc(buf[i]);
}
