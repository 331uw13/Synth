#ifndef GGUI_H
#define GGUI_H

#define GGUI_MOUSE_DOWN (1<<0)


struct ggui {
	double mouse_x;
	double mouse_y;
	int win_w;
	int win_h;
	int programs[8];
	int flags;
};

struct ggui* ggui_init();
void         ggui_quit(struct ggui* g);

int ggui_checkbox(struct ggui* g, double x, double y, int* ptr);
int ggui_knob    (struct ggui* g, double x, double y, double* ptr, double min, double max);



#endif
