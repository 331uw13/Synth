#include "gui.h"


void create_text(struct state_t* s, char* text, struct text_t** out) {
	if(text != NULL) {

		struct text_t* t = &s->gui.texts[s->gui.num_texts];

		SDL_Color col = { 180, 180, 180, 255 };
		SDL_Surface* surface = TTF_RenderText_Solid(s->font, text, col);
		t->texture = SDL_CreateTextureFromSurface(s->r, surface);
		
		SDL_FreeSurface(surface);
		TTF_SizeText(s->font, text, &t->rect.w, &t->rect.h);
		
		if(t->texture != NULL) {
			*out = t;
			s->gui.num_texts++;
		}
	}
}

void destroy_text(struct text_t* text) {
	SDL_DestroyTexture(text->texture);
}

void begin_frame(struct state_t* s, char* text) {	
	s->gui.frame_start_x = s->gui.pos_x;
	s->gui.frame_start_y = s->gui.pos_y;
}

void end_frame(struct state_t* s) {
	if(s->gui.num_boxes+1 < MAX_NUM_BOXES) {

		const int x = s->gui.frame_start_x*GUI_CELL_WIDTH+GUI_FRAME_OFFSET;
		const int y = s->gui.frame_start_y*GUI_CELL_HEIGHT+GUI_FRAME_OFFSET;
		const int w = (s->gui.pos_x - s->gui.frame_start_x)*GUI_CELL_WIDTH-GUI_FRAME_OFFSET*2;
		const int h = (s->gui.pos_y - s->gui.frame_start_y)*GUI_CELL_HEIGHT-GUI_FRAME_OFFSET*2;

		if(w > 0 && h > 0) {
			SDL_Rect* r = &s->gui.boxes[s->gui.num_boxes];
			r->x = x;
			r->y = y;
			r->w = w;
			r->h = h;
			s->gui.num_boxes++;
		}
	}
}

void add_knob_d(struct state_t* s, char* text, int color, double* ptr, double min, double max) {
	if(s->gui.num_knobs+1 < MAX_NUM_KNOBS) {

		struct knob_t* k = &s->gui.knobs[s->gui.num_knobs];
		k->data_type = DATA_TYPE_DOUBLE;
		k->color = color;
		k->ptr_d = ptr;
		k->min_d = min;
		k->max_d = max;

		k->x = s->gui.pos_x*GUI_CELL_WIDTH+GUI_ITEM_OFFSET/2;
		k->y = s->gui.pos_y*GUI_CELL_HEIGHT+GUI_ITEM_OFFSET/2;

		create_text(s, text, &k->text);

		s->gui.pos_y++;
		s->gui.num_knobs++;
	}
}

void add_knob_i(struct state_t* s, char* text, int color, int* ptr, int min, int max) {
	if(s->gui.num_knobs+1 < MAX_NUM_KNOBS) {

		struct knob_t* k = &s->gui.knobs[s->gui.num_knobs];
		k->data_type = DATA_TYPE_INT;
		k->color = color;
		k->ptr_i = ptr;
		k->min_i = min;
		k->max_i = max;

		k->x = s->gui.pos_x*GUI_CELL_WIDTH+GUI_ITEM_OFFSET/2;
		k->y = s->gui.pos_y*GUI_CELL_HEIGHT+GUI_ITEM_OFFSET/2;

		create_text(s, text, &k->text);

		s->gui.pos_y++;
		s->gui.num_knobs++;
	}
}

void add_output (struct state_t* s, double* ptr) {
	if(s->gui.num_outputs+1 < MAX_NUM_OUTPUTS) {
		struct wirept_t* p = &s->gui.outputs[s->gui.num_outputs];

		p->type = WIRE_TYPE_OUTPUT;
		p->out_ptr = ptr;
		p->color = 0xAA6060;
		p->x = s->gui.pos_x*GUI_CELL_WIDTH+GUI_CELL_WIDTH/2;
		p->y = s->gui.pos_y*GUI_CELL_HEIGHT+GUI_CELL_WIDTH/2;

		s->gui.pos_y++;
		s->gui.num_outputs++;
	}
}

void add_input  (struct state_t* s, double* ptr) {
}


/*
void add_output(struct state_t* s, double* ptr, int col, int row) {
	struct wirept_t* p = &s->outputs[s->num_outputs];

	p->x = col*10;
	p->y = row*10;

	p->type = WIRE_TYPE_OUTPUT;
	p->out_ptr = ptr;
	p->color = 0xAA6060;

	s->num_outputs++;
}
*/



