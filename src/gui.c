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
			if(out != NULL) {
				*out = t;
			}
			s->gui.num_texts++;
		}
	}
}

void destroy_text(struct text_t* text) {
	SDL_DestroyTexture(text->texture);
}

void add_frame(struct state_t* s, int x, int y, int w, int h) {
	if(s->gui.num_boxes+1 < MAX_NUM_BOXES) {
		
		x = x*GUI_CELL_WIDTH  + GUI_FRAME_OFFSET*2;
		y = y*GUI_CELL_HEIGHT + GUI_FRAME_OFFSET*2;
		w = w*GUI_CELL_WIDTH  - GUI_FRAME_OFFSET*2;
		h = h*GUI_CELL_HEIGHT - GUI_FRAME_OFFSET*2;

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

void add_knob_d(struct state_t* s, int x, int y, char* text, int color, double* ptr, double min, double max) {
	if(s->gui.num_knobs+1 < MAX_NUM_KNOBS) {

		struct knob_t* k = &s->gui.knobs[s->gui.num_knobs];
		k->data_type = DATA_TYPE_DOUBLE;
		k->color = color;
		k->ptr_d = ptr;
		k->min_d = min;
		k->max_d = max;

		k->x = x*GUI_CELL_WIDTH+GUI_ITEM_OFFSET/2+GUI_FRAME_OFFSET;
		k->y = y*GUI_CELL_HEIGHT+GUI_ITEM_OFFSET/2+GUI_FRAME_OFFSET;
		
		create_text(s, text, &k->text);
		s->gui.num_knobs++;
	}
}

void add_knob_i(struct state_t* s, int x, int y, char* text, int color, int* ptr, int min, int max) {
	if(s->gui.num_knobs+1 < MAX_NUM_KNOBS) {

		struct knob_t* k = &s->gui.knobs[s->gui.num_knobs];
		k->data_type = DATA_TYPE_INT;
		k->color = color;
		k->ptr_i = ptr;
		k->min_i = min;
		k->max_i = max;

		k->x = x*GUI_CELL_WIDTH+GUI_ITEM_OFFSET/2+GUI_FRAME_OFFSET;
		k->y = y*GUI_CELL_HEIGHT+GUI_ITEM_OFFSET/2+GUI_FRAME_OFFSET;

		create_text(s, text, &k->text);
		s->gui.num_knobs++;
	}
}



// NOTE: just have one function for 'add_output' and 'add_input' ... they are almost the same.

void add_output(struct state_t* s, int x, int y, double** ptr) {
	if(s->gui.num_outputs+1 < MAX_NUM_OUTPUTS) {
		struct wirept_t* p = &s->gui.outputs[s->gui.num_outputs];

		p->reserved = 0;
		p->type = WIRE_TYPE_OUTPUT;
		p->out_ptr = ptr;
		p->color = WIREPT_FREE_COLOR;
		p->x = x*GUI_CELL_WIDTH+GUI_CELL_WIDTH/2-15;
		p->y = y*GUI_CELL_HEIGHT+GUI_CELL_WIDTH/2-15;

		s->gui.num_outputs++;
	}
}

void add_input(struct state_t* s, int x, int y, double* ptr) {
	if(s->gui.num_outputs+1 < MAX_NUM_INPUTS) {
		struct wirept_t* p = &s->gui.inputs[s->gui.num_inputs];

		p->reserved = 0;
		p->type = WIRE_TYPE_INPUT;
		p->in_ptr = ptr;
		p->color = WIREPT_FREE_COLOR;
		p->x = x*GUI_CELL_WIDTH+GUI_CELL_WIDTH/2-15;
		p->y = y*GUI_CELL_HEIGHT+GUI_CELL_WIDTH/2-15;

		s->gui.num_inputs++;
	}
}




