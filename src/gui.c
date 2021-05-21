#include <stdio.h>

#include "gui.h"

#define SCALE 10


u8 gui_init(struct gui_state* g, u16 ww, u16 wh) {
	u8 result = 0;

	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
		fprintf(stderr, "ERROR: Failed to initialize SDL2! %s\n", SDL_GetError());
		goto finish;
	}

	/*
	if(TTF_Init() < 0) {
		fprintf(stderr, "ERROR: failed to initialize SDL2_ttf! %s\n", SDL_GetError());
		SDL_Quit();
		return -1;
	}
	*/

	g->w = SDL_CreateWindow("blabla", 
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, ww, wh, SDL_WINDOW_SHOWN);
	if(g->w == NULL) {
		SDL_Quit();
		fprintf(stderr, "ERROR: Failed to create window! %s\n", SDL_GetError());
		goto finish;
	}

	g->r = SDL_CreateRenderer(g->w, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if(g->r == NULL) {
		SDL_DestroyWindow(g->w);
		SDL_Quit();
		fprintf(stderr, "ERROR: Failed to create renderer! %s\n", SDL_GetError());
		goto finish;
	}

	SDL_ShowCursor(0);
	SDL_RenderSetScale(g->r, SCALE, SCALE);
	result = 1;

	g->cursor.x = 0;
	g->cursor.y = 0;
	g->bg.r = 200;
	g->bg.g = 200;
	g->bg.b = 200;
	g->bg.a = 255;

	g->fg.r = 25;
	g->fg.g = 25;
	g->fg.b = 25;
	g->fg.a = 255;

finish:
	return result;
}

void gui_quit(struct gui_state* g) {
	SDL_DestroyWindow(g->w);
	SDL_DestroyRenderer(g->r);
	SDL_Quit();
	puts("exit.");
	exit(EXIT_SUCCESS);
}

void gui_handle_event(struct gui_state* g, SDL_Event* e) {
	switch(e->type) {

		case SDL_QUIT:
			gui_quit(g);
			break;

		case SDL_KEYDOWN:
			switch(e->key.keysym.sym) {
				
				case 0x20:
					printf("Toggle\n");
					break;
		
				case ',':
					printf("-\n");
					break;
					
				case '.':
					printf("+\n");
					break;

				case 'w':
					g->cursor.y--;
					printf("Up    ^--\n");
					break;
		
				case 's':
					g->cursor.y++;
					printf("Down  v--\n");
					break;

				case 'a':
					g->cursor.x--;
					printf("Left  <--\n");
					break;
	
				case 'd':
					g->cursor.x++;
					printf("Right  -->\n");
					break;

			}
			break;

		default: break;
	}
}


void gui_update(struct gui_state* g) {
	SDL_SetRenderDrawColor(g->r,
		   	g->bg.r,
		   	g->bg.g,
		   	g->bg.b,
		   	255);

	SDL_RenderClear(g->r);

	SDL_SetRenderDrawColor(g->r,
		   	200,
		   	20,
		   	20,
		   	255);

	SDL_Rect r = { g->cursor.x, g->cursor.y, 1, 1 };
	SDL_RenderFillRect(g->r, &r);


	SDL_RenderPresent(g->r);
}



