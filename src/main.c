//
// Simple synthesizer with SDL2  (work in progress...)
// https://github.com/331uw13/Synth
//

#include "synth.h"

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
	SDL_Rect r = { x, y, size, size };
	SDL_RenderFillRect(s->r, &r);
	/*
	SDL_RenderSetScale(s->r, size, size);
	SDL_RenderDrawPoint(s->r, x/size, y/size);
	SDL_RenderSetScale(s->r, 1, 1);
	*/
}



void handle_event(struct state_t* s) {
	SDL_Event event;
	s->flags &= ~MOUSE_LEFT_DOWN;
	s->flags &= ~MOUSE_RIGHT_DOWN;

	while(SDL_PollEvent(&event)) {
		switch(event.type) {
	
			case SDL_MOUSEMOTION:
				s->mouse_x = event.motion.x;
				s->mouse_y = event.motion.y;
				break;

			case SDL_MOUSEBUTTONDOWN:
				if(event.button.button == SDL_BUTTON_RIGHT) {
					s->flags |= MOUSE_RIGHT_DOWN;
					s->flags &= ~DRAG_WIRE;
				}
				else if(event.button.button == SDL_BUTTON_LEFT){
					if(!(s->flags & MOUSE_LEFT_DOWN)) {
						s->mouse_down_x = s->mouse_x;
						s->mouse_down_y = s->mouse_y;
					}
					s->flags |= MOUSE_LEFT_DOWN;
				}
				break;
			
			case SDL_MOUSEBUTTONUP:
				s->focus_index = -1;
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

void render_text(struct state_t* s, struct text_t* t, int x, int y) {
	if(t != NULL) {
		t->rect.x = x;
		t->rect.y = y;
		SDL_RenderCopy(s->r, t->texture, NULL, &t->rect);
	}
}

void render_wire_point(struct state_t* s, u32 index) {
	struct wirept_t* p = &s->gui.outputs[index];
	if(p->type != WIRE_TYPE_NONE) {
		const u8 hover = AREA_OVERLAP(s->mouse_x, s->mouse_y, p->x-5, p->y-5, 20, 20);
		
		draw_point(s, p->x, p->y, 10, p->color);
		
	}
}

void render_knob(struct state_t* s, u32 index) {
	struct knob_t* k = &s->gui.knobs[index];

	const u8 hover = AREA_OVERLAP(
			s->mouse_x, s->mouse_y,
			k->x, k->y, KNOB_RADIUS*2, KNOB_RADIUS*2);
	const u8 used = hover && (s->flags & MOUSE_LEFT_DOWN);

	if(used && s->focus_index < 0) {
		s->focus_index = index;
	}

	const int col = (s->focus_index == index 
			|| (hover && s->focus_index < 0)) ? 0x606666 : 0x505555;

	const u8 int_data = (k->data_type == DATA_TYPE_INT);
	
	/*
	if(k->wire_point.in_ptr != NULL && k->wire_point.type == WIRE_TYPE_INPUT) {
		*k->ptr_d = *k->wire_point.in_ptr;
	}
	*/

	// I have modified it and made it little bit better(at least i think so..)
	// but the idea is same...
	// Here is the original one for "imgui knob widget":
	// https://github.com/ocornut/imgui/issues/942#issuecomment-268747260

	const double min = int_data ? (double)k->min_i : k->min_d;
	const double max = int_data ? (double)k->max_i : k->max_d;

	const double cx = k->x+KNOB_RADIUS;
	const double cy = k->y+KNOB_RADIUS;
	const double norm = normalize(int_data ? *k->ptr_i : *k->ptr_d, min, max);

	const double p = M_PI/4.0;
	double a = (M_PI-p)*norm*2.0+p;
	
	double x = -sin(a)*KNOB_RADIUS+cx;
	double y =  cos(a)*KNOB_RADIUS+cy;
	const double x2 = -sin(a)*KNOB_RADIUS*0.5+cx;
	const double y2 =  cos(a)*KNOB_RADIUS*0.5+cy;

	if(s->focus_index == index) {
		double a2 = M_PI+atan2(s->mouse_x-cx, cy-s->mouse_y);
		a2 = MAX(p, MIN(2.0*M_PI-p, a2));
		double new_value = lerp(0.5*(a2-p)/(M_PI-p), min, max);	
		double new_x = -sin(a2)*KNOB_RADIUS*1.2;

		if(!(((int_data ? *k->ptr_i : *k->ptr_d) <= min && new_x > 0) 
					|| ((int_data ? *k->ptr_i : *k->ptr_d) >= max && new_x < 0))) {
			if(int_data) { *k->ptr_i = (int)round(new_value); }
			else         { *k->ptr_d = new_value; }
		}
	}

	draw_circle(s, k->x, k->y, KNOB_RADIUS, 16, col);
	draw_line(s, x2, y2, x, y, k->color);
	
	render_text(s, k->text, cx-15, k->y+KNOB_RADIUS*2+k->text->rect.h/2);
	
	// ---------------------
	
	//render_wire_point(s, &k->wire_point);

	// Create half circle to rotate with line.

	const double r = KNOB_RADIUS/2.0;
	const double num = 16.0;
	const double step = (M_PI*2.0)/num;
	double x3 = k->x+r*2;
	double y3 = k->y+r*2;
	double t  = step+a;

	double x0 = x3+cos(t)*r;
	double y0 = y3+sin(t)*r;
	
	set_color(s, k->color);

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


	
	// DELETE LATER.
	for(int i = 0; i < s->gui.max_row; i++) {
		const int y = i*GUI_CELL_HEIGHT;
		draw_line(s, 0, y, s->window_w, y, 0x303030);
	}

	// DELETE LATER.
	for(int i = 0; i < s->gui.max_col; i++) {
		const int x = i*GUI_CELL_WIDTH;
		draw_line(s, x, 0, x, s->window_h, 0x303030);
	}
	

	set_color(s, 0x353535);
	SDL_RenderFillRects(s->r, s->gui.boxes, s->gui.num_boxes);

	for(u32 i = 0; i < s->gui.num_knobs; i++) {
		render_knob(s, i);
	}
	
	for(u32 i = 0; i < s->gui.num_outputs; i++) {
		render_wire_point(s, i);
	}

	/*
	if(s->flags & DRAG_WIRE) {
		if(w != NULL) {
			draw_line(s, w->x+5, w->y+5, s->mouse_x, s->mouse_y, w->color);
		}
	}
	*/

	//s->test_value = sin(s->time)*200.0+200.0;
}


void main_loop(struct state_t* s) {

	double dt = 0.0;  // delta time.
	const double min_dt = 20.0/1000.0;

	while(!(s->flags & SHOULD_QUIT)) {
		const double frame_start = SDL_GetPerformanceCounter();
		handle_event(s);

		SDL_SetRenderDrawColor(s->r, 30, 30, 30, 255);
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


void add_stuff(struct state_t* s) {
	
	//s->osc[0].output = &s->sound_output; // DELETE LATER.

	const int def_color = 0x30A030;
	const int osc_color = 0x20A0FF;

	static double tests[10];
	
	for(int i = 0; i < SYNTH_NUM_OSC; i++) {
		s->gui.pos_x = i;
		begin_frame(s, "");
		
		add_knob_d(s, "vol",    osc_color, &s->osc[i].vol,    0.0, 50.0);
		add_knob_d(s, "pitch",  osc_color, &s->osc[i].pitch,  0.0, 500.0);
		add_knob_i(s, "wave",   osc_color, &s->osc[i].waveform, 0, NUM_WAVE_OPTIONS);
		add_output(s, &tests[5]);
		add_output(s, &tests[5]);

		end_frame(s);
		s->gui.pos_y = 0;
	}

}


int main() {
	struct state_t* s = NULL;
	s = synth_init(44100, 512);
	
	if(s != NULL && !(s->flags & NOT_INITIALIZED)) {
		add_stuff(s);
		main_loop(s);
	}

	synth_quit(s);
	return 0;
}


