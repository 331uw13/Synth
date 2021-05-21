#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "synth.h"

// TODO: Optimize when everything else is done.



#define FONT_FILE "Topaz-8.ttf"
#define WINDOW_REQWIDTH  1100
#define WINDOW_REQHEIGHT 800
#define VOLUME_BAR_WIDTH 40
#define BUTTON_X_OFFSET 3     // Offset from left
#define BUTTON_Y_OFFSET 2     // Offset from up
#define BUTTON_X_SPACE  5     // Space between buttons X axis if in group.
#define BUTTON_Y_SPACE  5     // Space between buttons Y axis if in group.
#define BUTTON_BORDER   10    // Button size.
#define LINE_CUT_OFF 5

#define CIRCLE_SEGMENTS 10
#define ENVELOPE_BAR_WIDTH 150

#define MAX_ATTACK 1.5
#define MAX_DECAY 2.0
#define MAX_RELEASE 1.5



// By changing these values you can get different colors.
#define COLOR_DETAIL 15
#define COLOR_START  0
#define COLOR_SKIP   2

#define BG_R 40
#define BG_G 32
#define BG_B 30

#define BAR_R 62
#define BAR_G 45
#define BAR_B 35

#define BTN_R 92
#define BTN_G 65
#define BTN_B 35




static const int keyboard_layout[27] = {
	10,23,21,12,2,13,14,15,7,16,17,18,25,
	24,8,9,0,3,11,4,6,22,1,20,5,19
};

static const int PIANO_ORDER[27] = { 
	0,1,0,1,0,1,0,1,0,1,0,  1,0,1,0,1,0,1,0,1,0,  1,0,1,0,1
};


static const SDL_Color TEXT_NORMAL_COLOR  = { 245, 225, 200, 255 };
static const SDL_Color TEXT_OVERLAP_COLOR = { 230, 230, 230, 130 };


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


#define BTN_XOFF        (BUTTON_BORDER+BUTTON_X_SPACE)
#define BTN_YOFF        (BUTTON_BORDER+BUTTON_Y_SPACE)
#define BTN_START_XOFF  (BUTTON_X_OFFSET+BUTTON_BORDER)
#define BTN_START_YOFF  (BUTTON_Y_OFFSET+BUTTON_BORDER)
#define TEXT_NORMAL    0
#define TEXT_OVERLAY   1
#define SHOULD_QUIT    1

#define CLAMP(T, v, min, max)       ((T)(v < min ? min : v > max ? max : v))
#define LOOP_CLAMP(T, v, min, max)  ((T)(v < min ? max : v > max ? min : v))


static SDL_Window* sdlwindow;
static SDL_Renderer* sdlrenderer;


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
	double step = ((double)(input+COLOR_START)*COLOR_SKIP)/((double)COLOR_DETAIL);
	double h = LOOP_CLAMP(double, step, 0.0, 1.0);  // Hue
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

int illum(int v, int n) {
	return clamp(v+n, 0, 255);
}

u8 mouse_on_rect(struct state_t* s, SDL_Rect* r) {
	return (s->mouse_x >= r->x && s->mouse_y >= r->y && s->mouse_x <= r->x+r->w && s->mouse_y <= r->y+r->h);
}

void set_rect_center(SDL_Rect* a, SDL_Rect* b) {
	a->x = b->x+b->w/2-a->w/2;
	a->y = b->y+b->h/2-a->h/2;
}

void render_text(struct text_t* text) {
	SDL_RenderCopy(sdlrenderer, text->texture, NULL, &text->rect);
}

void create_text(struct text_t* text, TTF_Font* font, char* str, int x, int y, int sx, int sy, SDL_Color color) {
	SDL_Surface* surface = TTF_RenderText_Solid(font, str, color);
	text->texture = SDL_CreateTextureFromSurface(sdlrenderer, surface);
	SDL_FreeSurface(surface);
	TTF_SizeText(font, str, &text->rect.w, &text->rect.h);
	
	text->rect.x = x;
	text->rect.y = y;
	text->rect.w += sx;
	text->rect.h += sy;
}

void destroy_text(struct text_t* text) {
	SDL_DestroyTexture(text->texture);
}

void draw_line(u32 x0, u32 y0, u32 x1, u32 y1, struct color_t* col) {
	SDL_SetRenderDrawColor(sdlrenderer, col->r, col->g, col->b, 255);
	SDL_RenderDrawLine(sdlrenderer, x0, y0, x1, y1);
}

void draw_rect(SDL_Rect* r, struct color_t* col, int l) {
	if(l != 0) {
		col->r += clamp(col->r+l, 0, 255);
		col->g += clamp(col->g+l, 0, 255);
		col->b += clamp(col->b+l, 0, 255);
	}

	SDL_SetRenderDrawColor(sdlrenderer, col->r, col->g, col->b, 255);
	SDL_RenderFillRect(sdlrenderer, r);
}

u8 button(u32 x, u32 y, struct text_t* text, struct state_t* s) {
	SDL_Rect r = { x, y, text->rect.w+BUTTON_BORDER, text->rect.h+BUTTON_BORDER };
	u8 hover = mouse_on_rect(s, &r);

	set_rect_center(&text->rect, &r);

	SDL_SetRenderDrawColor(sdlrenderer, BTN_R, BTN_G, BTN_B, 255);
	SDL_RenderFillRect(sdlrenderer, &r);
	render_text(text);

	return s->mouse_button && hover;
}

u8 valuebar(struct valuebar_t* bar, struct state_t* s, struct color_t* col) {
	u8 changed = 0;
	u8 hover = mouse_on_rect(s, &bar->rect);

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
	else if(bar->grab_pos < 0) {
		bar->grab_pos = map_value(*bar->ptr, 
				horizontal ? bar->max : bar->min, horizontal ? bar->min : bar->max, y+h, y);
	}

	SDL_Rect fg;
	SDL_Rect grab_point;
	fg.x = bar->rect.x;
	
	if(horizontal) {
		fg.y = bar->rect.y;
		fg.w = bar->grab_pos - bar->rect.x;
		fg.h = bar->rect.h;
		grab_point.x = bar->grab_pos;
		grab_point.y = bar->rect.y;
		grab_point.w = 4;
		grab_point.h = bar->rect.h;
	}
	else {
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


#define TEXT_VOLUME  0
#define TEXT_ATTACK  1
#define TEXT_DECAY   2
#define TEXT_SUSTAIN 3
#define TEXT_RELEASE 4
#define TEXT_NOTE_OFFSET 5
#define TEXT_LFO_FREQ 6
#define TEXT_LFO_AMPL 7
// ...


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

	for(int i = 0; i < 26; i++) {
		state.keys_down[i] = 0;
	}

	struct text_t text_array[8];
	struct text_t wave_options[5];
	create_text(&wave_options[0], font, "SINE",     0,0,0,0, TEXT_NORMAL_COLOR);
	create_text(&wave_options[1], font, "SAW",      0,0,0,0, TEXT_NORMAL_COLOR);
	create_text(&wave_options[2], font, "SQUARE",   0,0,0,0, TEXT_NORMAL_COLOR);
	create_text(&wave_options[3], font, "TRIANGLE", 0,0,0,0, TEXT_NORMAL_COLOR);
	create_text(&wave_options[4], font, "WHITE_NOISE",    0,0,0,0, TEXT_NORMAL_COLOR);
	
	create_text(&text_array[TEXT_VOLUME], font, "START_VOLUME", 0, 0, 25,2, TEXT_OVERLAP_COLOR);
	create_text(&text_array[TEXT_ATTACK], font, "ATTACK", 0, 0, 25,2, TEXT_OVERLAP_COLOR);
	create_text(&text_array[TEXT_DECAY],   font, "DECAY",   0, 0, 25,2, TEXT_OVERLAP_COLOR);
	create_text(&text_array[TEXT_SUSTAIN], font, "SUSTAIN", 0, 0, 25,2, TEXT_OVERLAP_COLOR);
	create_text(&text_array[TEXT_RELEASE], font, "RELEASE", 0, 0, 25,2, TEXT_OVERLAP_COLOR);
	
	create_text(&text_array[TEXT_NOTE_OFFSET], font, "NOTE_OFFSET", 0, 0, 25,2, TEXT_OVERLAP_COLOR);
	create_text(&text_array[TEXT_LFO_FREQ], font, "LFO_FREQ", 0, 0, 25,2, TEXT_OVERLAP_COLOR);
	create_text(&text_array[TEXT_LFO_AMPL], font, "LFO_AMPL", 0, 0, 25,2, TEXT_OVERLAP_COLOR);


	struct color_t default_color = { 200, 80, 85 };
	struct color_t colors[SYNTH_NUM_OSC];
	for(int i = 0; i < SYNTH_NUM_OSC; i++) {
		create_color(i, &colors[i]);
	}

	struct valuebar_t volume_bar;
	struct valuebar_t volume_bar_osc[SYNTH_NUM_OSC];
	struct valuebar_t envelope_bars[SYNTH_NUM_OSC*4];
	struct valuebar_t effect_bars[SYNTH_NUM_OSC*3];

	create_valuebar(&volume_bar, 
			5, 5, VOLUME_BAR_WIDTH, state.wn_h-10,
		   	synth_get_master_vol(), 0.0, SYNTH_MAX_VOL);

	for(int i = 0; i < SYNTH_NUM_OSC; i++) {
		create_valuebar(&volume_bar_osc[i], 0, 0, 200, wave_options[0].rect.h+BUTTON_BORDER,
				&synth_osc(i)->vol, 0.0, SYNTH_MAX_VOL);
	
		create_valuebar(&effect_bars[i], 0, 0, 200, wave_options[0].rect.h+BUTTON_BORDER,
				&synth_osc(i)->note_offset, -12.0, 12.0);
	

		create_valuebar(&effect_bars[i+3], 0, 0, 200,
			   	wave_options[0].rect.h+BUTTON_BORDER,
				&synth_osc(i)->lfo_freq, 0.0, 5.0);

		create_valuebar(&effect_bars[i+6], 0, 0, 200,
			   	wave_options[0].rect.h+BUTTON_BORDER,
				&synth_osc(i)->lfo_ampl, 0.0, 1.0);


		
		create_valuebar(&envelope_bars[i], 0, 0, ENVELOPE_BAR_WIDTH,
			   	wave_options[0].rect.h+BUTTON_BORDER,
				&synth_env(i)->attack, 0.0, MAX_ATTACK);
		
		create_valuebar(&envelope_bars[i+3], 0, 0, ENVELOPE_BAR_WIDTH,
			   	wave_options[0].rect.h+BUTTON_BORDER,
				&synth_env(i)->decay, 0.0, MAX_DECAY);

		create_valuebar(&envelope_bars[i+6], 0, 0, ENVELOPE_BAR_WIDTH,
			   	wave_options[0].rect.h+BUTTON_BORDER,
				&synth_env(i)->sustain, 0.0, SYNTH_MAX_VOL);
		
		create_valuebar(&envelope_bars[i+9], 0, 0, ENVELOPE_BAR_WIDTH,
			   	wave_options[0].rect.h+BUTTON_BORDER,
				&synth_env(i)->release, 0.0, MAX_RELEASE);
	}

	double dt = 0.0;
	double start_time = 0.0;
	const double min_dt = 30.0 / 1000.0;
	const u32 num_wave_options = sizeof wave_options / sizeof *wave_options;
	const u32 num_texts = sizeof text_array / sizeof *text_array;

	//synth_set_paused(1);
	*synth_get_master_vol() = 0.20;


	double test = 0.0;
	const int circle_rad = 47;
	const int circle_x = state.wn_w-circle_rad*3;
	const int circle_y = (BTN_START_YOFF+BTN_YOFF)+
		SYNTH_NUM_OSC*volume_bar_osc[0].rect.h+30+circle_rad;


	SDL_Event event;
	while(!(state.flags & SHOULD_QUIT)) {
		start_time = SDL_GetPerformanceCounter();
		if(dt < min_dt) {
			SDL_Delay((min_dt-dt)*1000.0);
		}

		while(SDL_PollEvent(&event)) {
			int k = 0;
			switch(event.type) {
				case SDL_QUIT:
					state.flags |= SHOULD_QUIT;
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
					if(event.key.keysym.sym > 0x60 && event.key.keysym.sym < 0x7B) {
						k = keyboard_layout[event.key.keysym.sym-0x61];
						synth_set_key_on(k);
						state.keys_down[k] = 1;
					}
					break;

				case SDL_KEYUP:
					if(event.key.keysym.sym > 0x60 && event.key.keysym.sym < 0x7B) {
						k = keyboard_layout[event.key.keysym.sym-0x61];
						synth_set_key_off(k);
						state.keys_down[k] = 0;
					}
					break;

				default: break;
			}
		}


		SDL_SetRenderDrawColor(sdlrenderer, BG_R, BG_G, BG_B, 255);
		SDL_RenderClear(sdlrenderer);


		// Create visual effect.

		SDL_RenderSetScale(sdlrenderer, 5.0, 5.0);

		double theta = 0.0;
		double step = (2.0*M_PI)/CIRCLE_SEGMENTS;
		test += fabs(synth_get_previous_out()*M_PI*0.25)+0.0135;

		for(int i = 0; i < CIRCLE_SEGMENTS; i++) {
			
			double x0 = cos(test);
			double y0 = sin(test);
			float b = lerp((double)i/(double)CIRCLE_SEGMENTS, 0, 255);

			double x = circle_x + cos(theta+x0)*(circle_rad+x0);
			double y = circle_y + sin(theta+y0)*(circle_rad+y0);

		
			SDL_SetRenderDrawColor(sdlrenderer, 200, 80, 20, 255);
			SDL_RenderDrawPoint(sdlrenderer, x/5.0, y/5.0);
			
			SDL_SetRenderDrawColor(sdlrenderer, 230, 24, 30, 255);
			SDL_RenderDrawPoint(sdlrenderer, (x+5)/5.0, (y+5)/5.0);

			theta += step;
		}


		SDL_RenderSetScale(sdlrenderer, 1.0, 1.0);



		u32 x_off = 0;
		u32 y_off = 0;


		// --- Draw piano
		// NOTE: make this better later. For now its just for visualizing.

		int black_off = 0;

		int black_w = 20;
		int black_h = 25;

		int white_w = 25;
		int white_h = 50;

		int space = 3;


		// White keys
		for(int i = 0; i < 26; i++) {
				
			if(PIANO_ORDER[i]) {
				black_off++;
				continue;
			}

			SDL_Rect r;
			struct color_t col;

			col.r = 170;
			col.g = 170;
			col.b = 170;

			r.x = VOLUME_BAR_WIDTH+15+black_off*(white_w+space);
			r.y = state.wn_h-white_h-10;
			r.w = white_w;
			r.h = white_h;

			draw_rect(&r, state.keys_down[i] ? &default_color : &col, 0);
		}

		black_off = 0;


		// Black keys
		for(int i = 0; i < 26; i++) {
			
			if(!PIANO_ORDER[i]) {
				black_off++;
				continue;
			}

			SDL_Rect r;
			struct color_t col;

			
			col.r = 20;
			col.g = 20;
			col.b = 20;

			r.x = VOLUME_BAR_WIDTH+30+(black_off-1)*(white_w+space);
			r.y = state.wn_h - white_h - 10;
			r.w = black_w;
			r.h = black_h;

			draw_rect(&r, state.keys_down[i] ? &default_color : &col, 0);
		}


		x_off = 0;
		y_off = BTN_START_YOFF;


		// --- Wave options.

		for(u32 j = 0; j < SYNTH_NUM_OSC; j++) {
			struct color_t* col = &colors[j];
			x_off = VOLUME_BAR_WIDTH+BUTTON_BORDER*BUTTON_X_OFFSET;

			for(u32 i = 0; i < num_wave_options; i++) {
				const SDL_Rect* rect = &wave_options[i].rect;

				if(button(x_off, y_off, &wave_options[i], &state)) {
					synth_osc(j)->wave_type = i;
				}
				
				if(i == synth_osc(j)->wave_type) {
					SDL_SetRenderDrawColor(sdlrenderer, col->r, col->g, col->b, 255);
					SDL_Rect r = {
						x_off,
						y_off + rect->h+BUTTON_BORDER-3,
						rect->w + BUTTON_BORDER,
						3
					};
					SDL_RenderFillRect(sdlrenderer, &r);
				}
				
				x_off += rect->w + BTN_XOFF;
			}
			y_off += wave_options[0].rect.h + BTN_YOFF;
		}


		// --- Value bars

		valuebar(&volume_bar, &state, &default_color);
		
		for(int i = 0; i < SYNTH_NUM_OSC; i++) {
			struct valuebar_t* bar = &volume_bar_osc[i];
			bar->rect.x = x_off+BUTTON_X_OFFSET+30;
			bar->rect.y = BTN_START_YOFF+i*(bar->rect.h+BUTTON_Y_SPACE);
			valuebar(bar, &state, &colors[i]);
			set_rect_center(&text_array[TEXT_VOLUME].rect, &bar->rect);
			render_text(&text_array[TEXT_VOLUME]);

			bar = &effect_bars[i];
			bar->rect.x = x_off+BUTTON_X_OFFSET+30 + bar->rect.w+30;
			bar->rect.y = BTN_START_YOFF+i*(bar->rect.h+BUTTON_Y_SPACE);
			valuebar(bar, &state, &colors[i]);
			set_rect_center(&text_array[TEXT_NOTE_OFFSET].rect, &bar->rect);
			render_text(&text_array[TEXT_NOTE_OFFSET]);
		}
		
		
		x_off = VOLUME_BAR_WIDTH+BUTTON_BORDER*BUTTON_X_OFFSET;

		for(int i = 0; i < SYNTH_NUM_OSC; i++) {
			for(int j = 0; j < 4; j++) {
				struct valuebar_t* bar = &envelope_bars[i+(j*3)];	
				bar->rect.x = x_off+30+(bar->rect.w+35)*j;
				bar->rect.y = BTN_START_YOFF+20+(i+SYNTH_NUM_OSC)*(bar->rect.h+BUTTON_Y_SPACE);
				valuebar(bar, &state, &colors[i]);
				set_rect_center(&text_array[TEXT_ATTACK+j].rect, &bar->rect);
				render_text(&text_array[TEXT_ATTACK+j]);
			}


			struct valuebar_t* bar = &effect_bars[i+3];	
			bar->rect.x = x_off;
			bar->rect.y = BTN_START_YOFF+40+(i+SYNTH_NUM_OSC*2)*(bar->rect.h+BUTTON_Y_SPACE);
			valuebar(bar, &state, &colors[i]);
			set_rect_center(&text_array[TEXT_LFO_FREQ].rect, &bar->rect);
			render_text(&text_array[TEXT_LFO_FREQ]);

			bar = &effect_bars[i+6];	
			bar->rect.x = x_off;
			bar->rect.y = BTN_START_YOFF+40+(i+SYNTH_NUM_OSC*3)*(bar->rect.h+BUTTON_Y_SPACE);
			valuebar(bar, &state, &colors[i]);
			set_rect_center(&text_array[TEXT_LFO_AMPL].rect, &bar->rect);
			render_text(&text_array[TEXT_LFO_AMPL]);


		}
	

		SDL_RenderPresent(sdlrenderer);
		state.mouse_wheel = 0;

		dt = (SDL_GetPerformanceCounter()-start_time)/SDL_GetPerformanceFrequency();
	}

	for(u32 i = 0; i < num_wave_options; i++) {
		destroy_text(&wave_options[i]);
	}

	for(u32 i = 0; i < num_texts; i++) {
		destroy_text(&text_array[i]);
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


