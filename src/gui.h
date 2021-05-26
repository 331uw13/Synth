#ifndef GUI_H
#define GUI_H

#include "things.h"


void create_text(struct state_t* s, char* text, struct text_t** out);
void destroy_text(struct text_t* text);

void add_knob_i(struct state_t* s, char* text,    int* ptr,    int min,    int max, int col, int row, int color);
void add_knob_f(struct state_t* s, char* text, double* ptr, double min, double max, int col, int row, int color, u8 has_input_point);

void add_box(struct state_t* s, int col, int row, int col_w, int row_h);

void add_output_point(struct state_t* s, double* out_ptr, int col, int row);





#endif
