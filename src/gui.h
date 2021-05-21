#ifndef GUI_H
#define GUI_H

#include <SDL2/SDL.h>

typedef long unsigned int   u64;
typedef unsigned int        u32;
typedef unsigned short      u16;
typedef unsigned char       u8;



struct v2 {
	u32 x;
	u32 y;
};

struct gui_state {
	SDL_Window*    w;
	SDL_Renderer*  r;
	SDL_Color    bg;
	SDL_Color    fg;
	struct v2    cursor;
};

u8   gui_init(struct gui_state* g, u16 ww, u16 wh);
void gui_quit(struct gui_state* g);
void gui_handle_event(struct gui_state* g, SDL_Event* e);
void gui_update(struct gui_state* g);



#endif
