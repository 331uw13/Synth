#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "synth.h"


#define FONT_FILE "Topaz-8.ttf"
#define WINDOW_REQWIDTH 1000
#define WINDOW_REQHEIGHT 500
#define SHOULDQUIT (1<<0)

#define VOLUME_BAR_WIDTH 30
#define BUTTON_X_OFFSET 5
#define BUTTON_Y_OFFSET 2
#define BUTTON_X_SPACE  3
#define BUTTON_Y_SPACE  3
#define BUTTON_BORDER 5


struct state_t {
	int flags;
	u32 mouse_x;
	u32 mouse_y;
	int mouse_button;
	int mouse_wheel;
	int wn_w;    // Window width
	int wn_h;    // Window height
};

struct text_t {
	SDL_Texture* texture;
	SDL_Rect rect;
};

struct value_bar_t {
	u32 x;
	u32 y;
	u32 w;
	u32 h;
	double min;
	double max;
	double* value;
	int mouse_y;
};


static SDL_Window* sdlwindow;
static SDL_Renderer* sdlrenderer;


void update(struct state_t* state) {
	const u8* keys = SDL_GetKeyboardState(0);


	// Uhh....

	if(keys[SDL_SCANCODE_A]) {
		synth_playkey(0);
	}
	else if(keys[SDL_SCANCODE_S]) {
		synth_playkey(1);
	}
	else if(keys[SDL_SCANCODE_D]) {
		synth_playkey(2);
	}
	else if(keys[SDL_SCANCODE_F]) {
		synth_playkey(3);
	}
	else if(keys[SDL_SCANCODE_D]) {
		synth_playkey(4);
	}
	else if(keys[SDL_SCANCODE_G]) {
		synth_playkey(5);
	}
	else if(keys[SDL_SCANCODE_H]) {
		synth_playkey(6);
	}
	else if(keys[SDL_SCANCODE_J]) {
		synth_playkey(7);
	}
	else if(keys[SDL_SCANCODE_K]) {
		synth_playkey(8);
	}
	else if(keys[SDL_SCANCODE_L]) {
		synth_playkey(9);
	}
	else {
		synth_set_hz(0.0, 0);
		synth_set_hz(0.0, 1);
		synth_set_hz(0.0, 2);
	}
}

void render_text(struct text_t* text) {
	SDL_RenderCopy(sdlrenderer, text->texture, NULL, &text->rect);
}

void create_text(struct text_t* text, TTF_Font* font, char* str, int x, int y) {
	SDL_Color col = { 230,220,220,255 };
	SDL_Surface* surface = TTF_RenderText_Solid(font, str, col);
	text->texture = SDL_CreateTextureFromSurface(sdlrenderer, surface);
	SDL_FreeSurface(surface);

	text->rect.x = x;
	text->rect.y = y;
	TTF_SizeText(font, str, &text->rect.w, &text->rect.h);
}

void destroy_text(struct text_t* text) {
	SDL_DestroyTexture(text->texture);
}

int clamp(int a, int min, int max) {
	return (a <= min) ? min : (a >= max) ? max : a;
}

double lerp(double a, double min, double max) {
	return (max-min)*a+min;
}

double normalize(double a, double min, double max) {
	return (a-min)/(max-min);
}

double map_value(double a, double src_min, double src_max, double dest_min, double dest_max) {
	return lerp(normalize(a, src_min, src_max), dest_min, dest_max);
}

u8 button(u32 x, u32 y, struct text_t* text, struct state_t* s) {

	text->rect.x = x;
	text->rect.y = y;
	u32 rx = x - BUTTON_BORDER;
	u32 ry = y - BUTTON_BORDER;
	u32 rw = text->rect.w + BUTTON_BORDER*2;
	u32 rh = text->rect.h + BUTTON_BORDER*2;

	SDL_Rect r = { rx, ry, rw, rh };
	u8 hover = (s->mouse_x >= rx && s->mouse_y >= ry && s->mouse_x <= rx+rw && s->mouse_y <= ry+rh);
	
	SDL_SetRenderDrawColor(sdlrenderer, hover ? 140 : 80, 45, 30, 255);
	SDL_RenderFillRect(sdlrenderer, &r);
	render_text(text);

	return s->mouse_button && hover;
}

u8 value_bar(struct value_bar_t* bar, struct state_t* s) {

	SDL_Rect bg = { bar->x, bar->y, bar->w, bar->h };
	u8 changed = 0;
	u8 hover = (s->mouse_x >= bar->x && s->mouse_y >= bar->y 
			&& s->mouse_x <= bar->x+bar->w && s->mouse_y <= bar->y+bar->h);

	if(hover) {
		bar->mouse_y = clamp(s->mouse_button ? s->mouse_y : (u32)(bar->mouse_y - s->mouse_wheel), bar->y, bar->y+bar->h);
		double new_value = map_value(bar->mouse_y, bar->y, bar->y+bar->h, bar->max, bar->min);
		changed = (*bar->value != new_value);
		*bar->value = new_value;
	}
	else if(bar->mouse_y < 0) {
		bar->mouse_y = map_value(*bar->value, bar->min, bar->max, bar->y+bar->h, bar->y);
	}

	SDL_Rect fg = {
		bar->x,
		bar->mouse_y + 2,
	   	bar->w,
	   	bar->h - (bar->mouse_y - bar->y)
	};
	
	SDL_Rect drag = {
		bar->x,
		bar->mouse_y,
	   	bar->w,
		4
	};

	SDL_SetRenderDrawColor(sdlrenderer, 75, 55, 40, 255);
	SDL_RenderFillRect(sdlrenderer, &bg);

	double t = normalize(*bar->value, bar->min, bar->max);
	u8 r = (u8)lerp(t, 30,  200);
	u8 g = (u8)lerp(t, 200, 80);
	u8 b = (u8)lerp(t, 30,  30);

	SDL_SetRenderDrawColor(sdlrenderer, r, g, b, 255);
	SDL_RenderFillRect(sdlrenderer, &fg);
	
	SDL_SetRenderDrawColor(sdlrenderer, r+50, g+50, b+50, 255);
	SDL_RenderFillRect(sdlrenderer, &drag);


	return changed;
}



void main_loop() {
	struct state_t state;
	state.flags = 0;
	state.mouse_button = 0;
	
	SDL_GetWindowSize(sdlwindow, &state.wn_w, &state.wn_h);
	TTF_Font* font = TTF_OpenFont(FONT_FILE, 16);
	if(font == NULL) {
		fprintf(stderr, "Failed to open ttf font: %s\n", SDL_GetError());
	}

	struct text_t wave_options[5];
	create_text(&wave_options[0], font, "SINE",     0, 0);
	create_text(&wave_options[1], font, "SAW",      0, 0);
	create_text(&wave_options[2], font, "SQUARE",   0, 0);
	create_text(&wave_options[3], font, "TRIANGLE", 0, 0);
	create_text(&wave_options[4], font, "NOISE",    0, 0);


	double test_value = 0.0;
	struct value_bar_t volume_bar = { 
		5, 5, VOLUME_BAR_WIDTH, state.wn_h-10,
		0.0, SYNTH_MAX_VOL, synth_get_master_vol(), -1
   	};


	double dt = 0.0;
	double start_time = 0.0;
	double last_time = 0.0;
	const double min_dt = 20.0 / 1000.0;
	const u32 num_wave_options = sizeof wave_options / sizeof *wave_options;

	//synth_set_paused(1);

	SDL_Event event;
	while(!(state.flags & SHOULDQUIT)) {
		start_time = SDL_GetPerformanceCounter();
		if(dt < min_dt) {
			SDL_Delay((min_dt-dt)*1000.0);
		}


		while(SDL_PollEvent(&event)) {
			switch(event.type) {
				case SDL_QUIT:
					state.flags |= SHOULDQUIT;
					break;

				case SDL_MOUSEWHEEL:
					state.mouse_wheel = event.wheel.y;
					break;

				case SDL_MOUSEBUTTONDOWN:
					state.mouse_button = event.button.button;
					break;
				
				case SDL_MOUSEBUTTONUP:
					state.mouse_button = 0;
					break;

				case SDL_MOUSEMOTION:
					state.mouse_x = event.motion.x;
					state.mouse_y = event.motion.y;
					break;

				default: break;
			}
		}

		SDL_SetRenderDrawColor(sdlrenderer, 35, 25, 25, 255);
		SDL_RenderClear(sdlrenderer);

		u32 x_off = VOLUME_BAR_WIDTH+BUTTON_BORDER*BUTTON_X_OFFSET;
		u32 y_off = BUTTON_BORDER*BUTTON_Y_OFFSET;

		// --- Wave options.

		//render_text(&wave_options[0]);

		for(u32 j = 0; j < SYNTH_NUM_OSC; j++) {
			for(u32 i = 0; i < num_wave_options; i++) {
				if(button(x_off, y_off, &wave_options[i], &state)) {
					synth_set_osc_type(i, j);
				}

				if(i == synth_get_osc_type(j)) {
					SDL_SetRenderDrawColor(sdlrenderer, 210, 85, 35, 255);
					SDL_Rect r = {
						x_off-BUTTON_BORDER,
						(y_off-5)+wave_options[i].rect.h+BUTTON_BORDER*2-3,
						wave_options[i].rect.w+BUTTON_BORDER*2,
						3
					};
					SDL_RenderFillRect(sdlrenderer, &r);
				}

				x_off += wave_options[i].rect.w+BUTTON_BORDER*BUTTON_X_SPACE;
			}

			x_off = VOLUME_BAR_WIDTH+BUTTON_BORDER*BUTTON_X_OFFSET;
			y_off += wave_options[0].rect.h + BUTTON_BORDER*BUTTON_Y_SPACE;
		}


		// --- Value bars

		value_bar(&volume_bar, &state);


		update(&state);
		SDL_RenderPresent(sdlrenderer);
		state.mouse_wheel = 0;

		dt = (SDL_GetPerformanceCounter()-start_time)/SDL_GetPerformanceFrequency();
	}

	for(u32 i = 0; i < num_wave_options; i++) {
		destroy_text(&wave_options[i]);
	}

	TTF_CloseFont(font);
}


void free_memory_and_exit() {
	synth_quit();
	TTF_Quit();
	SDL_DestroyWindow(sdlwindow);
	SDL_DestroyRenderer(sdlrenderer);
	SDL_Quit();
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

	if(TTF_Init() < 0) {
		fprintf(stderr, "failed to initialize SDL2_ttf! %s\n", SDL_GetError());
		SDL_Quit();
		return -1;
	}

	sdlwindow = SDL_CreateWindow("Synthesizer", 
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

	synth_init();
	main_loop();
	free_memory_and_exit();
	return 0;
}


