#include <string.h>

#include "ggui.h"
#include "shader.h"
#include "gui_shaders.h"

#include <GL/gl.h>
#include <math.h>


#define CHECKBOX_INDEX  0
#define BUTTON_INDEX    2
#define KNOB_INDEX      3

#define MIN(a, b) ((a < b) ? a : b)
#define MAX(a, b) ((a > b) ? a : b)



static void   _render(struct ggui* g, double x, double y, double w, double h, int program, double value);
static int    _area_hovered(struct ggui* g, double x, double y, double w, double h);
static double _normalize (double t, double min, double max);
static double _lerp      (double t, double min, double max);
static double _map       (double t, double s_min, double s_max, double d_min, double d_max);



struct ggui* ggui_init() {
	struct ggui* g = malloc(sizeof *g);
	if(g == NULL) {
		fprintf(stderr, "ggui(ERROR): Failed to allocate %li bytes of memory!\n", sizeof *g);
		goto finish;
	}
	
	g->flags = 0;
	g->mouse_x = 0.0;
	g->mouse_y = 0.0;
	g->win_w = 0;
	g->win_h = 0;
	memset(g->programs, 0, sizeof g->programs);

	int tmp[2];
	tmp[0] = compile_shader(GGUI_VERTEX_SHADER, GL_VERTEX_SHADER);

	tmp[1] = compile_shader(GGUI_CHECKBOX_SHADER, GL_FRAGMENT_SHADER);
	g->programs[CHECKBOX_INDEX] = create_program(tmp, 2);
	glDeleteShader(tmp[1]);

	tmp[1] = compile_shader(GGUI_KNOB_SHADER, GL_FRAGMENT_SHADER);
	g->programs[KNOB_INDEX] = create_program(tmp, 2);
	glDeleteShader(tmp[1]);


finish:
	return g;
}


void ggui_quit(struct ggui* g) {
	if(g != NULL) {
		for(unsigned int i = 0; i < (sizeof g->programs / sizeof *g->programs); i++) {
			if(g->programs[i] > 0) {
				glDeleteProgram(g->programs[i]);
			}	
		}
		free(g);
	}
}

int ggui_checkbox(struct ggui* g, double x, double y, int* ptr) {
	int used = 0;
	if(ptr != NULL) {
		_render(g, x, y, 30, 30, g->programs[CHECKBOX_INDEX], (double)*ptr);
		if(_area_hovered(g, x, y, 30, 30) && (g->flags & GGUI_MOUSE_DOWN)) {
			*ptr = !*ptr;
			used = 1;
		}
	}
	return used;
}


int ggui_knob(struct ggui* g, double x, double y, double* ptr, double max, double min) {
	int used = 0;
	if(ptr != NULL) {
		_render(g, x, y, 55, 55, g->programs[KNOB_INDEX], *ptr);
		
		if(_area_hovered(g, x, y, 55, 55) && (g->flags & GGUI_MOUSE_HOLD_DOWN)) {
			double p = M_PI/4.0;
			double a = M_PI+atan2(g->mouse_x-x, y-g->mouse_y);
			a = MAX(p, MIN(2.0*M_PI-p, a));
			*ptr = _lerp(0.5*(a-p)/(M_PI-p), min, max);
			used = 1;
		}
	}

	return used;
}



int _area_hovered(struct ggui* g, double x, double y, double w, double h) {
	return (g->mouse_x >= x-w/2 && g->mouse_x <= (x-w/2)+w 
			&& g->mouse_y >= y-h/2 && g->mouse_y <= (y-h/2)+h);
}

void _render(struct ggui* g, double x, double y, double w, double h, int program, double value) {

	glUseProgram(program);

	// TODO: optimize later.

	glUniform2f(glGetUniformLocation(program, "pos"),  x, y);
	glUniform2f(glGetUniformLocation(program, "size"), w, h);
	glUniform2f(glGetUniformLocation(program, "win_size"), g->win_w, g->win_h);
	glUniform2f(glGetUniformLocation(program, "mouse"), g->mouse_x, g->mouse_y);
	glUniform1f(glGetUniformLocation(program, "value"), value);
	
	x = _map(x, 0.0, g->win_w,  -1.0,  1.0);
	y = _map(y, 0.0, g->win_h,   1.0, -1.0);
	w = _map(w, 0.0, g->win_w,   0.0,  1.0);
	h = _map(h, 0.0, g->win_h,   0.0,  1.0);
	
	glBegin(GL_QUADS);
	glVertex2f(x-w,   y+h);
	glVertex2f(x-w,   y-h);
	glVertex2f(x+w,   y-h);
	glVertex2f(x+w,   y+h);
	glEnd();

}

double _normalize(double t, double min, double max) {
	return (t-min)/(max-min);
}

double _lerp(double t, double min, double max) {
	return (max-min)*t+min;
}

double _map(double t, double s_min, double s_max, double d_min, double d_max) {
	return _lerp(_normalize(t, s_min, s_max), d_min, d_max);
}

