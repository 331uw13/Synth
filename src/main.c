#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "synth.h"


#define FONT_FILE "Topaz-8.ttf"
#define WINDOW_REQWIDTH  1100
#define WINDOW_REQHEIGHT 500
#define VOLUME_BAR_WIDTH 40
#define BUTTON_X_OFFSET 5
#define BUTTON_Y_OFFSET 2
#define BUTTON_X_SPACE  3
#define BUTTON_Y_SPACE  3
#define BUTTON_BORDER 5
#define COLOR_DETAIL 10
#define COLOR_OFFSET 3

#define BG_R 40
#define BG_G 32
#define BG_B 30

#define BAR_R 62
#define BAR_G 45
#define BAR_B 35

#define BTN_R 92
#define BTN_G 65
#define BTN_B 35


// TODO: Optimize text rendering and other stuff when everything else is done.


static const u8 keyboard_layout[26] = {
	16, 22, 17, 19, 24, 20, 8, 14, 15,
	0, 18, 3, 5, 6, 7, 9, 10, 11, 25,
	23, 2, 21, 1, 13, 12
};

struct state_t {
	int flags;
	u8 keys_down[26];
	int mouse_x;
	int mouse_y;
	int mouse_button;  // 1 if any mouse key is pressed.
	int mouse_wheel;   // Negative value when mouse wheel is scrolled down, positive if up else zero.
	int wn_w;      // Window width.
	int wn_h;      // Window height.
};

struct text_t {
	SDL_Texture* texture;
	SDL_Rect rect;
};

struct valuebar_t {
	SDL_Rect rect;
	double min;
	double max;
	double* ptr;
	int grab_pos;
};

struct color_t {
	u8 r;
	u8 g;
	u8 b;
};


#define SHOULDQUIT (1<<0)


static SDL_Window* sdlwindow;
static SDL_Renderer* sdlrenderer;


void update(struct state_t* state) {
	const u8* keys = SDL_GetKeyboardState(0);


	// Uhh....
	// TODO: make this better.
	
	if(keys[SDL_SCANCODE_A]) {
		synth_playkey(0);
	}
	else if(keys[SDL_SCANCODE_S]) {
		synth_playkey(2);
	}
	else if(keys[SDL_SCANCODE_D]) {
		synth_playkey(4);
	}
	else if(keys[SDL_SCANCODE_F]) {
		synth_playkey(5);
	}
	else if(keys[SDL_SCANCODE_D]) {
		synth_playkey(7);
	}
	else if(keys[SDL_SCANCODE_G]) {
		synth_playkey(9);
	}
	else if(keys[SDL_SCANCODE_H]) {
		synth_playkey(10);
	}
	else if(keys[SDL_SCANCODE_J]) {
		synth_playkey(12);
	}
	else if(keys[SDL_SCANCODE_K]) {
		synth_playkey(14);
	}
	else if(keys[SDL_SCANCODE_L]) {
		synth_playkey(15);
	}
	else {
		for(int i = 0; i < SYNTH_NUM_OSC; i++) {
			synth_set_hz(0.0, i);
		}
	}
}

int clamp(int a, int min, int max) {
	return ((a <= min) ? min : (a >= max) ? max : a);
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

double get_hue(double p, double q, double t) {
	t += (t < 0.0 || t > 1.0) ? -t*0.5 : 0.0;
	return (t < 0.15) ? p+(q-p)*6.0*t : (t < 0.5) ? q : p+(q-p)*(0.7-t)*6.0;
}

void create_color(int input, struct color_t* color_out) {
	double step = ((double)input*COLOR_OFFSET)/((double)COLOR_DETAIL);
	double h = step;  // Hue
	double s = 0.35;  // Saturation
	double l = 0.5;   // Lightness

	double q = (l < 0.5) ? l*(1.0+s) : l+s-l*s;
	double p = 2.0*l-q;

	int r = round(get_hue(p, q, h+0.35) * 255.0);
	int g = round(get_hue(p, q, h)      * 255.0);
	int b = round(get_hue(p, q, h-0.35) * 255.0);
	color_out->r = clamp(r, 0, 255);
	color_out->g = clamp(g, 0, 255);
	color_out->b = clamp(b, 0, 255);
}

// illuminate color
// can be used to dim color if n is negative value.
int illum(int v, int n) {
	return clamp(v+n, 0, 255);
}

void render_text(struct text_t* text) {
	SDL_RenderCopy(sdlrenderer, text->texture, NULL, &text->rect);
}

void create_text(struct text_t* text, TTF_Font* font, char* str, int x, int y) {
	SDL_Color col = { 245, 225, 200, 255 };
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

u8 button(u32 x, u32 y, struct text_t* text, struct state_t* s) {
	text->rect.x = x + BUTTON_BORDER;
	text->rect.y = y + BUTTON_BORDER;
	int rx = x;
	int ry = y;
	int rw = text->rect.w + BUTTON_BORDER*2;
	int rh = text->rect.h + BUTTON_BORDER*2;

	SDL_Rect r = { rx, ry, rw, rh };
	u8 hover = (s->mouse_x >= rx && s->mouse_y >= ry && s->mouse_x <= rx+rw && s->mouse_y <= ry+rh);
	
	SDL_SetRenderDrawColor(sdlrenderer, BTN_R, BTN_G, BTN_B, 255);
	SDL_RenderFillRect(sdlrenderer, &r);
	render_text(text);

	return s->mouse_button && hover;
}

u8 valuebar(struct valuebar_t* bar, struct state_t* s, struct color_t* col) {
	u8 changed = 0;
	u8 hover = (s->mouse_x >= bar->rect.x && s->mouse_y >= bar->rect.y 
			&& s->mouse_x <= bar->rect.x + bar->rect.w && s->mouse_y <= bar->rect.y + bar->rect.h);

	int y = bar->rect.y;
	int h = bar->rect.h;
	int m = s->mouse_y;
	int wheel = s->mouse_wheel;

	const u8 horizontal = (bar->rect.w > bar->rect.h);
	double new_value = 0.0;
	
	if(horizontal) {
		y = bar->rect.x;
		h = bar->rect.w;
		m = s->mouse_x;
		wheel = -s->mouse_wheel;
	}

	if(hover) {
		bar->grab_pos = clamp(s->mouse_button ? m : (bar->grab_pos - wheel), y, y+h);
		new_value = map_value(bar->grab_pos, y, y+h, 
				horizontal ? bar->min : bar->max, horizontal ? bar->max : bar->min);
		changed = (*bar->ptr != new_value);
		*bar->ptr = new_value;
	}
	else {
		bar->grab_pos = map_value(*bar->ptr, 
				horizontal ? bar->max : bar->min, horizontal ? bar->min : bar->max, y+h, y);
	}

	SDL_Rect fg;
	SDL_Rect grab_point;
	
	if(horizontal) {

		fg.x = bar->rect.x;
		fg.y = bar->rect.y;
		fg.w = bar->grab_pos - bar->rect.x;
		fg.h = bar->rect.h;

		grab_point.x = bar->grab_pos;
		grab_point.y = bar->rect.y;
		grab_point.w = 4;
		grab_point.h = bar->rect.h;
	}
	else {
		
		fg.x = bar->rect.x;
		fg.y = bar->grab_pos;
		fg.w = bar->rect.w;
		fg.h = bar->rect.y - bar->grab_pos + bar->rect.h;

		grab_point.x = bar->rect.x;
		grab_point.y = bar->grab_pos;
		grab_point.w = bar->rect.w;
		grab_point.h = 4;
	}


	SDL_SetRenderDrawColor(sdlrenderer, BAR_R, BAR_G, BAR_B, 255);
	SDL_RenderFillRect(sdlrenderer, &bar->rect);

	SDL_SetRenderDrawColor(sdlrenderer, col->r, col->g, col->b, 255);
	SDL_RenderFillRect(sdlrenderer, &fg);
	
	SDL_SetRenderDrawColor(sdlrenderer, illum(col->r, 50), illum(col->g, 50), illum(col->b, 50), 255);
	SDL_RenderFillRect(sdlrenderer, &grab_point);


	return changed;
}


void create_valuebar(struct valuebar_t* bar, u32 x, u32 y, u32 w, u32 h, double* ptr, double min, double max) {
	bar->rect.x = x;
	bar->rect.y = y;
	bar->rect.w = w;
	bar->rect.h = h;
	bar->ptr = ptr;
	bar->min = min;
	bar->max = max;
	bar->grab_pos = -1;
}


void main_loop() {
	struct state_t state;
	state.flags = 0;
	state.mouse_button = 0;
	
	SDL_GetWindowSize(sdlwindow, &state.wn_w, &state.wn_h);
	TTF_Font* font = TTF_OpenFont(FONT_FILE, 16);
	if(font == NULL) {
		fprintf(stderr, "ERROR: %s\n", SDL_GetError());
		return;
	}

	struct text_t wave_options[5];
	create_text(&wave_options[0], font, "SINE",     0, 0);
	create_text(&wave_options[1], font, "SAW",      0, 0);
	create_text(&wave_options[2], font, "SQUARE",   0, 0);
	create_text(&wave_options[3], font, "TRIANGLE", 0, 0);
	create_text(&wave_options[4], font, "NOISE",    0, 0);


	struct color_t default_color = { 200, 80, 80 };
	struct color_t colors[SYNTH_NUM_OSC];
	for(int i = 0; i < SYNTH_NUM_OSC; i++) {
		create_color(i, &colors[i]);
	}


	struct valuebar_t volume_bar;
	struct valuebar_t volume_bar_osc[SYNTH_NUM_OSC];
	
	create_valuebar(&volume_bar, 
			5, 5, VOLUME_BAR_WIDTH, state.wn_h-10,
		   	synth_get_master_vol(), 0.0, SYNTH_MAX_VOL);

	for(int i = 0; i < SYNTH_NUM_OSC; i++) {
		create_valuebar(&volume_bar_osc[i],
				0, 0,
				200, wave_options[0].rect.h+BUTTON_BORDER*2,
				&synth_get_osc(i)->vol, 0.0, SYNTH_MAX_VOL);
	}

	double dt = 0.0;
	double start_time = 0.0;
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

				case SDL_KEYDOWN:
					break;

				case SDL_KEYUP:
					break;

				default: break;
			}
		}

		SDL_SetRenderDrawColor(sdlrenderer, BG_R, BG_G, BG_B, 255);
		SDL_RenderClear(sdlrenderer);



		u32 x_off = 0;
		u32 y_off = BUTTON_BORDER*BUTTON_Y_OFFSET;

		// --- Wave options.

		for(u32 j = 0; j < SYNTH_NUM_OSC; j++) {
			struct color_t* col = &colors[j];
			x_off = VOLUME_BAR_WIDTH+BUTTON_BORDER*BUTTON_X_OFFSET;
			for(u32 i = 0; i < num_wave_options; i++) {
				if(button(x_off, y_off, &wave_options[i], &state)) {
					synth_set_osc_type(i, j);
				}
				if(i == synth_get_osc_type(j)) {
					SDL_SetRenderDrawColor(sdlrenderer, col->r, col->g, col->b, 255);
					SDL_Rect r = {
						x_off,
						y_off+wave_options[i].rect.h+BUTTON_Y_OFFSET*BUTTON_BORDER-3,
						wave_options[i].rect.w+BUTTON_BORDER*2,
						3
					};
					SDL_RenderFillRect(sdlrenderer, &r);
				}
				x_off += wave_options[i].rect.w+BUTTON_BORDER*BUTTON_X_SPACE;
			}
			y_off += wave_options[0].rect.h+BUTTON_BORDER*BUTTON_Y_SPACE;
		}


		// --- Value bars

		valuebar(&volume_bar, &state, &default_color);
		
		for(int i = 0; i < SYNTH_NUM_OSC; i++) {
			struct valuebar_t* bar = &volume_bar_osc[i];

			bar->rect.x = x_off+BUTTON_X_OFFSET+30;
			bar->rect.y = BUTTON_Y_OFFSET*BUTTON_BORDER+i*(bar->rect.h+BUTTON_BORDER*BUTTON_Y_SPACE-BUTTON_BORDER*2);

			valuebar(bar, &state, &colors[i]);
		}
		

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
		fprintf(stderr, "ERROR: Failed to initialize SDL2! %s\n", SDL_GetError());
		return -1;
	}

	if(TTF_Init() < 0) {
		fprintf(stderr, "ERROR: failed to initialize SDL2_ttf! %s\n", SDL_GetError());
		SDL_Quit();
		return -1;
	}

	sdlwindow = SDL_CreateWindow("Synthesizer", 
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			WINDOW_REQWIDTH, WINDOW_REQHEIGHT,
			SDL_WINDOW_SHOWN);

	if(sdlwindow == NULL) {
		SDL_Quit();
		fprintf(stderr, "ERROR: Failed to create window! %s\n", SDL_GetError());
		return -1;
	}

	sdlrenderer = SDL_CreateRenderer(sdlwindow, -1, SDL_RENDERER_ACCELERATED);
	if(sdlrenderer == NULL) {
		SDL_DestroyWindow(sdlwindow);
		SDL_Quit();
		fprintf(stderr, "ERROR: Failed to create renderer! %s\n", SDL_GetError());
		return -1;
	}

	synth_init();
	main_loop();
	free_memory_and_exit();
	return 0;
}


