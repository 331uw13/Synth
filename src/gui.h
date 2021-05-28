#ifndef GUI_H
#define GUI_H

#include "things.h"


void create_text(struct state_t* s, char* text, struct text_t** out);
void destroy_text(struct text_t* text);


void gui_item_position(struct state_t* s, int* x, int* y);


void begin_frame(struct state_t* s, char* text);
void end_frame(struct state_t* s);

void add_knob_d (struct state_t* s, char* text, int color, double* ptr, double min, double max);
void add_knob_i (struct state_t* s, char* text, int color,    int* ptr,    int min,    int max);

void add_output (struct state_t* s, double* ptr);
void add_input  (struct state_t* s, double* ptr);




#endif
