#ifndef TERM_GRAPHICS_H
#define TERM_GRAPHICS_H

#include <stdio.h>

#define TERMG_PRNTBUFSIZE 1024

#define TERMG_RED     1
#define TERMG_ORANGE  208
#define TERMG_GREEN   40
#define TERMG_BLUE    27
#define TERMG_PURPLE  165
#define TERMG_CYAN    80
#define TERMG_YELLOW  220

#define TERMG_BLACK 0
#define TERMG_DARK 235
#define TERMG_GRAY 244
#define TERMG_WHITE 255

#define TERMG_BG 0
#define TERMG_FG 1


typedef unsigned int    u32;
typedef unsigned short  u16;
typedef unsigned char   u8;


void termg_init();
void termg_quit();
void termg_clear();
void termg_clear_line(u16 y);
void termg_win_size(u16* w, u16* h);
void termg_color(u8 flag, u16 color);
void termg_print(u16 x, u16 y, const char* str, ...);



#endif
