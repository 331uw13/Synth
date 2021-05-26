#include "gui.h"


void create_text(struct state_t* s, char* text, struct text_t** out) {
	if(text != NULL) {

		struct text_t* t = &s->texts[s->num_texts];

		SDL_Color col = { 180, 180, 180, 255 };
		SDL_Surface* surface = TTF_RenderText_Solid(s->font, text, col);
		t->texture = SDL_CreateTextureFromSurface(s->r, surface);
		
		SDL_FreeSurface(surface);
		TTF_SizeText(s->font, text, &t->rect.w, &t->rect.h);
		
		if(t->texture != NULL) {
			*out = t;
			s->num_texts++;
		}
	}
}

void destroy_text(struct text_t* text) {
	SDL_DestroyTexture(text->texture);
}


void add_box(struct state_t* s, int col, int row, int col_w, int row_h) {
	SDL_Rect* r = &s->boxes[s->num_boxes];

	r->x = col*GUI_ITEM_X_OFFSET+GUI_ITEM_X_SPACE-KNOB_RADIUS/2;
	r->y = row*GUI_ITEM_Y_OFFSET+GUI_ITEM_Y_SPACE-KNOB_RADIUS/2;
	
	r->w = col_w*GUI_ITEM_X_OFFSET;
	r->h = row_h*GUI_ITEM_Y_OFFSET-3;

	s->num_boxes++;
}

void add_knob_f(struct state_t* s, char* text, double* ptr, double min, double max, int col, int row, 
		int color, u8 has_input_point) {

	struct knob_t* k = &s->knobs[s->num_knobs];

	k->data_type = DATA_TYPE_DOUBLE;
	k->x = col*GUI_ITEM_X_OFFSET+GUI_ITEM_X_SPACE;
	k->y = row*GUI_ITEM_Y_OFFSET+GUI_ITEM_Y_SPACE;
	k->ptr_d = ptr;
	k->min_d = min;
	k->max_d = max;
	k->color = color;

	if(has_input_point) {
		k->wire_point.type = WIRE_TYPE_INPUT;
		k->wire_point.x = k->x+KNOB_RADIUS;
		k->wire_point.y = k->y+KNOB_RADIUS*2+20;
		k->wire_point.color = 0x60AAAA;
	}
	else {
		k->wire_point.type = WIRE_TYPE_NONE;
	}

	k->wire_point.in_ptr = NULL;
	k->wire_point.out_ptr = NULL;

	create_text(s, text, &k->text);
	s->num_knobs++;
}

void add_knob_i(struct state_t* s, char* text, int* ptr, int min, int max, int col, int row, int color) {
	struct knob_t* k = &s->knobs[s->num_knobs];

	k->data_type = DATA_TYPE_INT;
	k->x = col*GUI_ITEM_X_OFFSET+GUI_ITEM_X_SPACE;
	k->y = row*GUI_ITEM_Y_OFFSET+GUI_ITEM_Y_SPACE;
	k->ptr_i = ptr;
	k->min_i = min;
	k->max_i = max;
	k->color = color;

	k->wire_point.type = WIRE_TYPE_NONE;
	k->wire_point.in_ptr = NULL;
	k->wire_point.out_ptr = NULL;

	create_text(s, text, &k->text);
	s->num_knobs++;
}

void add_output_point(struct state_t* s, double* out_ptr, int col, int row) {
	struct wire_point_t* p = &s->output_points[s->num_output_points];

	p->x = col*10;
	p->y = row*10;

	p->type = WIRE_TYPE_OUTPUT;
	p->out_ptr = out_ptr;
	p->color = 0xAA6060;

	s->num_output_points++;
}



