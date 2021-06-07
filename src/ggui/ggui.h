#ifndef GGUI_H
#define GGUI_H

#define GGUI_MOUSE_DOWN       (1<<0)
#define GGUI_MOUSE_HOLD_DOWN  (1<<1)



struct ggui {
	double mouse_x;
	double mouse_y;
	int win_w;
	int win_h;
	int programs[8];
	int flags;
	unsigned int font_texture;
	unsigned int font_data_ubo;

};

struct ggui* ggui_init();
void         ggui_quit(struct ggui* g);

int ggui_button  (struct ggui* g, double x, double y, int always_color);
int ggui_checkbox(struct ggui* g, double x, double y, unsigned char* ptr);
int ggui_knob_d  (struct ggui* g, double x, double y, double* ptr, double min, double max);
int ggui_knob_i  (struct ggui* g, double x, double y,    int* ptr,    int min,    int max);

void ggui_text(struct ggui* g, double x, double y, double width, char* text);



#endif
