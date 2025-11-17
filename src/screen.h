#ifndef SCREEN_H
#define SCREEN_H

#define VGA_W 80
#define VGA_H 25
#define VGA_COLOR 0x07
static volatile unsigned short *const VGA = (unsigned short*)0xB8000;

void putc(char ch);
void puts(const char *s);
void print_uint(unsigned int x);
void scroll_if_needed(void);

#endif
