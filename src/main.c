//
// Simple synthesizer with SDL2  (work in progress...)
// https://github.com/331uw13/Synth
//

#include "synth.h"
#include "gui.h"

#define GET_R(r) (r>>16)
#define GET_G(g) (g>>8)
#define GET_B(b) (b>>0)


void set_color(struct state_t* s, int color) {
	SDL_SetRenderDrawColor(s->r, GET_R(color), GET_G(color), GET_B(color), 255);
}

void draw_circle(struct state_t* s, double x, double y, double r, int num_s, int color) {
	const double step = (M_PI*2.0)/num_s;
	x += r;
	y += r;
	double x0 = x+1.0*r;
	double y0 = y;
	double t  = 0.0;
	
	set_color(s, color);

	for(int i = 0; i < num_s+1; i++) {
		const double x1 = x0;
		const double y1 = y0;
		x0 = x+cos(t)*r;
		y0 = y+sin(t)*r;
		t += step;
		SDL_RenderDrawLine(s->r, x1, y1, x0, y0);
	}
}

void draw_line(struct state_t* s, double x0, double y0, double x1, double y1, int color) {
	set_color(s, color);
	SDL_RenderDrawLine(s->r, x1, y1, x0, y0);
}

void draw_point(struct state_t* s, double x, double y, int size, int color) {
	set_color(s, color);
	SDL_RenderSetScale(s->r, size, size);
	SDL_RenderDrawPoint(s->r, x/size, y/size);
	SDL_RenderSetScale(s->r, 1, 1);
}



void handle_event(struct state_t* s) {
	SDL_Event event;

	while(SDL_PollEvent(&event)) {
		switch(event.type) {
	
			case SDL_MOUSEMOTION:
				s->mouse_x = event.motion.x;
				s->mouse_y = event.motion.y;
				break;

			case SDL_MOUSEBUTTONDOWN:
				/*
				s->mouse_down_x = s->mouse_x;
				s->mouse_down_y = s->mouse_y;
				s->flags |= RESTORE_MOUSE_POS;
				*/
				s->flags |= MOUSE_DOWN;
				//SDL_ShowCursor(0);
				break;
			
			case SDL_MOUSEBUTTONUP:
				s->focus_index = -1;
				/*
				if(s->flags & RESTORE_MOUSE_POS) {
					SDL_WarpMouseInWindow(s->w, s->mouse_down_x, s->mouse_down_y);
					s->flags &= ~RESTORE_MOUSE_POS;
				}
				*/
				s->flags &= ~MOUSE_DOWN;
				//SDL_ShowCursor(1);
				break;
			
			case SDL_QUIT:
				s->flags |= SHOULD_QUIT;
				break;

			default: break;
		}
	}
}

double normalize(double v, double min, double max) {
	return (v-min)/(max-min);
}

double lerp(double v, double min, double max) {
	return (max-min)*v+min;
}

void render_knob(struct state_t* s, int index) {
	struct knob_t* k = &s->knobs[index];

	const u8 hover = AREA_OVERLAP(
			s->mouse_x, s->mouse_y,
			k->center_x, k->center_y, KNOB_RADIUS*2, KNOB_RADIUS*2);
	const u8 used = hover && (s->flags & MOUSE_DOWN);

	if(used && s->focus_index < 0) {
		s->focus_index = index;
	}

	const int col = (s->focus_index == index 
			|| (hover && s->focus_index < 0)) ? 0xA0A0A0 : 0x808080;


	// I have modified it and made it little bit better(at least i think so..)
	// but the idea is same...
	// Here is the original one for "imgui knob widget":
	// https://github.com/ocornut/imgui/issues/942#issuecomment-268747260

	const double cx = k->center_x+KNOB_RADIUS;
	const double cy = k->center_y+KNOB_RADIUS;
	const double norm = normalize(*k->ptr, k->min, k->max);
	const double p = M_PI/4.0;
	double a = (M_PI-p)*norm*2.0+p;
	
	double x = -sin(a)*KNOB_RADIUS*1.2+cx;
	double y =  cos(a)*KNOB_RADIUS*1.2+cy;
	const double x2 = -sin(a)*KNOB_RADIUS*0.5+cx;
	const double y2 =  cos(a)*KNOB_RADIUS*0.5+cy;

	if(s->focus_index == index) {
		a = M_PI+atan2(s->mouse_x-cx, cy-s->mouse_y);
		a = MAX(p, MIN(2.0*M_PI-p, a));
		*k->ptr = lerp(0.5*(a-p)/(M_PI-p), k->min, k->max);
	}

	const double r = KNOB_RADIUS/2.0;

	draw_circle(s, k->center_x, k->center_y, KNOB_RADIUS, 16, col);
	draw_circle(s, k->center_x+r, k->center_y+r, r, 16, 0x303030);

	draw_line(s, x2, y2, x, y, 0x00FFFF);
	draw_line(s, x2, y2, x, y, 0x00FFFF);

	// ---------------------


	// Create half circle to rotate with line.
	
	const double num = 16.0;
	const double step = (M_PI*2.0)/num;
	double x3 = k->center_x+r*2;
	double y3 = k->center_y+r*2;
	double t  = step+a;

	double x0 = x3+cos(t)*r;
	double y0 = y3+sin(t)*r;
	
	set_color(s, 0x107070);

	for(int i = 0; i < (num/2)-1; i++) {
		const double x1 = x0;
		const double y1 = y0;
		x0 = x3+cos(t)*r;
		y0 = y3+sin(t)*r;
		t += step;
		SDL_RenderDrawLine(s->r, x1, y1, x0, y0);
	}
}


void render(struct state_t* s) {
	for(int i = 0; i < s->num_knobs; i++) {
		render_knob(s, i);
	}
}


void main_loop(struct state_t* s) {

	double dt = 0.0;  // delta time.
	const double min_dt = 30.0 / 1000.0;

	while(!(s->flags & SHOULD_QUIT)) {
		const double frame_start = SDL_GetPerformanceCounter();
		handle_event(s);
	
		SDL_SetRenderDrawColor(s->r, 20, 20, 20, 255);
		SDL_RenderClear(s->r);

		render(s);
		SDL_RenderPresent(s->r);

		dt = (SDL_GetPerformanceCounter()-frame_start)/SDL_GetPerformanceFrequency();
		if(dt < min_dt) {
			// Frame finished early.
			SDL_Delay((min_dt-dt)*1000.0);
		}
	}
}

static double test_value0 = 0.0;
static double test_value1 = 0.0;


void add_stuff(struct state_t* s) {

	s->next_knob_x = 200;
	s->next_knob_y = 200;

	add_knob(s, "test_knob0", &test_value0, 0.0, 15.0);
	add_knob(s, "test_knob1", &test_value1, -3.0, 3.0);

}



int main() {
	struct state_t* s = NULL;
	s = synth_init(44100, 512);
	
	if(s == NULL || (s->flags & NOT_INITIALIZED)) {
		return -1;
	}

	add_stuff(s);
	main_loop(s);
	synth_quit(s);
	return 0;
}

