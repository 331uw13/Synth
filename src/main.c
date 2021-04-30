#include <stdio.h>
#include <SDL2/SDL.h>

#include "synth.h"

#define BASEFREQ 110.0
#define WINDOW_REQWIDTH 800
#define WINDOW_REQHEIGHT 500
#define SHOULDQUIT (1<<0)


struct state_t {
	int flags;
	int m_x;
	int m_y;
	int wn_w;
	int wn_h;
	// ...
};

static SDL_Window* sdlwindow;
static SDL_Renderer* sdlrenderer;
static SDL_Point* wavebuffer;
static u32 wavebuffer_size;

static double hz;


void sound(double t, double* out) {
	*out = sin(2*M_PI * hz * t);
	if(*out > 0.0) {
		*out = 0.1;
	}
	else {
		*out = -0.1;
	}

	*out *= 8000;
}


void update(struct state_t* state) {
	const u8* keys = SDL_GetKeyboardState(0);


	// https://www.youtube.com/watch?v=OSCzKOqtgcA

}


void keydown(u8 key, struct state_t* state) {
}


void main_loop() {
	SDL_Event event;
	struct state_t state;
	
	SDL_GetWindowSize(sdlwindow, &state.wn_w, &state.wn_h);
	wavebuffer = malloc(state.wn_w * sizeof *wavebuffer);
	wavebuffer_size = state.wn_w;
	if(wavebuffer == NULL) {
		fprintf(stderr, "Failed to allocate memory for 'wavebuffer'\n");
		return;
	}

	while(!(state.flags & SHOULDQUIT)) {
	
		while(SDL_PollEvent(&event)) {
			switch(event.type) {
				case SDL_QUIT:
					state.flags |= SHOULDQUIT;
					break;

				case SDL_KEYDOWN:
					if(!event.key.repeat) {
						keydown(event.key.keysym.sym, &state);
					}
					break;

				default: break;
			}
		}

		SDL_SetRenderDrawColor(sdlrenderer, 25, 25, 25, 255);
		SDL_RenderClear(sdlrenderer);
		
		update(&state);

		SDL_RenderPresent(sdlrenderer);
	}
}


void free_memory_and_exit() {
	synth_quit();
	SDL_DestroyWindow(sdlwindow);
	SDL_DestroyRenderer(sdlrenderer);
	SDL_Quit();
	if(wavebuffer != NULL) {
		free(wavebuffer);
	}
	puts("exit.");
	exit(EXIT_SUCCESS);
}

int main() {
	sdlwindow = NULL;
	sdlrenderer = NULL;

	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
		fprintf(stderr, "Failed to initialize SDL2! %s\n", SDL_GetError());
		return -1;
	}

	sdlwindow = SDL_CreateWindow("learning about synthesizers", 
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			WINDOW_REQWIDTH, WINDOW_REQHEIGHT,
			SDL_WINDOW_SHOWN);

	if(sdlwindow == NULL) {
		SDL_Quit();
		fprintf(stderr, "Failed to create window! %s\n", SDL_GetError());
		return -1;
	}

	sdlrenderer = SDL_CreateRenderer(sdlwindow, -1, SDL_RENDERER_ACCELERATED);
	if(sdlrenderer == NULL) {
		SDL_DestroyWindow(sdlwindow);
		SDL_Quit();
		fprintf(stderr, "Failed to create renderer! %s\n", SDL_GetError());
		return -1;
	}

	synth_init(44100, 512, 1);
	synth_callback(sound);

	main_loop();
	free_memory_and_exit();
	return 0;
}


