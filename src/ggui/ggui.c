#include <string.h>
#include <stddef.h>

#include "ggui.h"
#include "shader.h"
#include "gui_shaders.h"

#include "stb_font_consolas_49_usascii.h"

#include <GL/gl.h>
#include <math.h>


#define FONT_INDEX      0
#define CHECKBOX_INDEX  1
#define BUTTON_INDEX    2
#define KNOB_INDEX      3

#define MIN(a, b) ((a < b) ? a : b)
#define MAX(a, b) ((a > b) ? a : b)

typedef unsigned int  u32;



static void   _render(struct ggui* g, double x, double y, double w, double h, double value, int program);
static int    _area_hovered(struct ggui* g, double x, double y, double w, double h);
static double _normalize (double t, double min, double max);
static double _lerp      (double t, double min, double max);
static double _map       (double t, double s_min, double s_max, double d_min, double d_max);


// !
static stb_fontchar font_data[STB_SOMEFONT_NUM_CHARS];



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

	// Create bitmap font texture.

	static unsigned char font_pixels[STB_SOMEFONT_BITMAP_HEIGHT][STB_SOMEFONT_BITMAP_WIDTH];
	STB_SOMEFONT_CREATE(font_data, font_pixels, STB_SOMEFONT_BITMAP_HEIGHT);


	glGenTextures(1, &g->font_texture);
	glBindTexture(GL_TEXTURE_2D, g->font_texture);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, STB_SOMEFONT_BITMAP_WIDTH, STB_SOMEFONT_BITMAP_HEIGHT, 0,
		   	GL_RED, GL_UNSIGNED_BYTE, font_pixels);
	glBindTexture(GL_TEXTURE_2D, 0);



	int tmp[2];
	tmp[0] = compile_shader(GGUI_VERTEX_SHADER, GL_VERTEX_SHADER);

	// Create shader for font.
	tmp[1] = compile_shader(GGUI_FONT_SHADER, GL_FRAGMENT_SHADER);
	g->programs[FONT_INDEX] = create_program(tmp, 2);
	glDeleteShader(tmp[1]);


	// Using uniform buffer to store characters position, width and height.
	
	const u32 num_chars = STB_SOMEFONT_NUM_CHARS;
	const u32 ubo_size = num_chars*(sizeof(float)*4); 
	
	printf("%i\n", num_chars);

	glGenBuffers(1, &g->font_data_ubo);
	glBindBuffer(GL_UNIFORM_BUFFER, g->font_data_ubo);
	glBufferData(GL_UNIFORM_BUFFER, ubo_size, NULL, GL_STATIC_DRAW);

	void* ubo_ptr = glMapBuffer(GL_UNIFORM_BUFFER, GL_READ_WRITE);
	if(ubo_ptr != NULL) {
		const u32 vec4_size = sizeof(float)*4;
		for(u32 i = 0; i < num_chars; i++) {
			memmove(ubo_ptr+(i*vec4_size), &font_data[i].s0f, vec4_size);
		}
	}
	else {
		fprintf(stderr, "ggui(ERROR): Failed to map uniform buffer to store font data.\n");
	}

	glUnmapBuffer(GL_UNIFORM_BUFFER);
	glBindBufferRange(GL_UNIFORM_BUFFER, 0, g->font_data_ubo, 0, ubo_size);
	
	glBindBuffer(GL_UNIFORM_BUFFER, 0);




	// Create shader for checkboxes.
	tmp[1] = compile_shader(GGUI_CHECKBOX_SHADER, GL_FRAGMENT_SHADER);
	g->programs[CHECKBOX_INDEX] = create_program(tmp, 2);
	glDeleteShader(tmp[1]);

	// Create shader for knobs.
	tmp[1] = compile_shader(GGUI_KNOB_SHADER, GL_FRAGMENT_SHADER);
	g->programs[KNOB_INDEX] = create_program(tmp, 2);
	glDeleteShader(tmp[1]);

	glDeleteShader(tmp[0]);


finish:
	return g;
}

void ggui_text(struct ggui* g, double x, double y, char* text) {
	

	const int program = g->programs[FONT_INDEX];

		
	glUseProgram(program);
	
	glBindTexture(GL_TEXTURE_2D, g->font_texture);
	glActiveTexture(GL_TEXTURE0);
	

	/*
	stb_fontchar* cd = &font_data['A'-STB_SOMEFONT_FIRST_CHAR];
	
	glUniform4f(glGetUniformLocation(program, "glyph_pos"),
		   	cd->s0f, cd->t0f,
			cd->s1f, cd->t1f);
	*/

	_render(g, x, y, 500, 500, 0.0, program);





	/*
	while(*text) {
		int char_codepoint = *text++;
		stb_fontchar* cd = &font_data[char_codepoint-STB_SOMEFONT_FIRST_CHAR];
		
		double _x[3] = {
			_map(x,       0.0, g->win_w,  -0.5,  0.5),
			_map(cd->x0f, 0.0, g->win_w,  -0.5,  0.5),
			_map(cd->x1f, 0.0, g->win_w,  -0.5,  0.5)
		};

		double _y[3] = {
			_map(y,       0.0, g->win_h,   0.5, -0.5),
			_map(cd->y0f, 0.0, g->win_h,   0.5, -0.5),
			_map(cd->y1f, 0.0, g->win_h,   0.5, -0.5)
		};


		float points[] = {
			_x[0]+_x[2], _y[0]+_y[2],  cd->s1f, cd->t1f,
			_x[0]+_x[1], _y[0]+_y[2],  cd->s0f, cd->t1f,
			_x[0]+_x[2], _y[0]+_y[1],  cd->s1f, cd->t0f,
			_x[0]+_x[1], _y[0]+_y[1],  cd->s0f, cd->t0f
		};

		glBindBuffer(GL_ARRAY_BUFFER, g->quad_vbo);
		void* ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);
		
		if(ptr != NULL) {

			memmove(ptr, points, sizeof points);
			glUnmapBuffer(GL_ARRAY_BUFFER);

			//glBufferData(GL_ARRAY_BUFFER, sizeof points, points, GL_STREAM_DRAW);
			
			glBindVertexArray(g->quad_vao);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

			x += cd->advance;
		}

	}
	*/
}


void ggui_quit(struct ggui* g) {
	if(g != NULL) {
		for(unsigned int i = 0; i < (sizeof g->programs / sizeof *g->programs); i++) {
			if(g->programs[i] > 0) {
				glDeleteProgram(g->programs[i]);
			}	
		}
		glDeleteTextures(1, &g->font_texture);
		free(g);
	}
}

int ggui_checkbox(struct ggui* g, double x, double y, int* ptr) {
	int used = 0;
	if(ptr != NULL) {
		const int program = g->programs[CHECKBOX_INDEX];
		glUseProgram(program);
		_render(g, x, y, 30, 30, (double)*ptr, program);
		
		if(_area_hovered(g, x, y, 30, 30) && (g->flags & GGUI_MOUSE_DOWN)) {
			*ptr = !*ptr;
			used = 1;
		}
	}
	return used;
}


int ggui_knob_d(struct ggui* g, double x, double y, double* ptr, double min, double max) {
	int used = 0;
	if(ptr != NULL) {
		const int program = g->programs[KNOB_INDEX];
		glUseProgram(program);
		glUniform2f(glGetUniformLocation(program, "limits"), min, max);
		
		_render(g, x, y, 55, 55, *ptr, program);

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

int ggui_knob_i(struct ggui* g, double x, double y, int* ptr, int min, int max) {
	int used = 0;
	if(ptr != NULL) {
		const int program = g->programs[KNOB_INDEX];
		glUseProgram(program);
		glUniform2f(glGetUniformLocation(program, "limits"), min, max);
		
		_render(g, x, y, 55, 55, *ptr, program);

		if(_area_hovered(g, x, y, 55, 55) && (g->flags & GGUI_MOUSE_HOLD_DOWN)) {
			double p = M_PI/4.0;
			double a = M_PI+atan2(g->mouse_x-x, y-g->mouse_y);
			a = MAX(p, MIN(2.0*M_PI-p, a));
			*ptr = (int)round(_lerp(0.5*(a-p)/(M_PI-p), min, max));
			used = 1;
		}
	}

	return used;
}


int _area_hovered(struct ggui* g, double x, double y, double w, double h) {
	return (g->mouse_x >= x-w/2 && g->mouse_x <= (x-w/2)+w 
			&& g->mouse_y >= y-h/2 && g->mouse_y <= (y-h/2)+h);
}

void _render(struct ggui* g, double x, double y, double w, double h, double value, int program) {

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

