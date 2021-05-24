#include "gui.h"


void add_knob(struct state_t* s, char* text, double* ptr, double min, double max) {

	struct knob_t* k = &s->knobs[s->num_knobs];

	k->center_x = s->next_knob_x;
	k->center_y = s->next_knob_y;
	k->ptr = ptr;
	k->min = min;
	k->max = max;

	s->next_knob_y += KNOB_RADIUS*2+30;
	s->num_knobs++;

}
