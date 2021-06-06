#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <SDL2/SDL.h>

#include "synth.h"
#include "util.h"



static void glfw_mouse_button_callback(GLFWwindow* w, int button, int act, int mods);
static void glfw_cursor_position_callback(GLFWwindow* w, double x, double y);
static void sdl_audio_callback(void* userdata, u8* stream, int bytes);



struct state_t*  synth_init() {
	struct state_t* s = malloc(sizeof *s);
	
	if(s == NULL) {
		fprintf(stderr, "ERROR: Failed to allocate %li bytes of memory! errno: %i\n", 
				sizeof *s, errno);

		goto finish;
	}

	s->flags = 0;
	s->w = NULL;
	s->gui = NULL;

	// Initialize GLFW and OpenGL.

	glfwInitHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwInitHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwInitHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
	
	if(!glfwInit()) {
		fprintf(stderr, "ERROR: Failed to initialize glfw!\n");
		s->flags |= NOT_INITIALIZED;
		goto finish;
	}

	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	
	if((s->w = glfwCreateWindow(1200, 850, WINDOW_TITLE, NULL, NULL)) == NULL) {
		fprintf(stderr, "ERROR: Failed to create window!\n");
		s->flags |= NOT_INITIALIZED;
		goto finish;
	}

	glfwMakeContextCurrent(s->w);
	glfwSwapInterval(1);

	if(glewInit()) {
		fprintf(stderr, "ERROR: Failed to initialize glew!\n");
		s->flags |= NOT_INITIALIZED;
		goto finish;
	}

	if((s->gui = ggui_init()) == NULL) {
		fprintf(stderr, "ERROR: Failed to initialize gui!\n");
		s->flags |= NOT_INITIALIZED;
		goto finish;
	}

	glfwSetWindowUserPointer(s->w, s->gui);
	glfwGetWindowSize(s->w, &s->gui->win_w, &s->gui->win_h);

	glfwSetMouseButtonCallback(s->w, glfw_mouse_button_callback);
	glfwSetCursorPosCallback(s->w,   glfw_cursor_position_callback);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	// Initialize SDL for audio.

	if(SDL_Init(SDL_INIT_AUDIO) < 0) {
		fprintf(stderr, "ERROR: Failed to initialize SDL for audio!\n%s\n", SDL_GetError());
		s->flags |= NOT_INITIALIZED;
		goto finish;
	}
	

	for(int i = 0; i < SYNTH_NUM_OSC; i++) {
		struct osc_t* o = &s->synth.osc[i];
		o->volume = 0.5;
		o->waveform = W_SINE;
		o->tune = 130.0;
		o->detail = 50.0;
	}

	s->synth.time = 0.0;
	s->synth.time_pos = 0.0;

	SDL_AudioSpec asreq;
	
	asreq.freq = 44100;
	asreq.samples = 512;
	asreq.channels = 1;
	asreq.format = AUDIO_S16SYS;
	asreq.callback = sdl_audio_callback;
	asreq.userdata = &s->synth;

	if(SDL_OpenAudio(&asreq, &s->synth.audio) < 0) {
		fprintf(stderr, "ERROR: Failed to open audio device!\n%s\n", SDL_GetError());
		s->flags |= NOT_INITIALIZED;
		goto finish;
	}

	
	//SDL_PauseAudio(0);



finish:
	return s;
}

void synth_quit(struct state_t* s) {
	if(s != NULL) {
		if(s->w != NULL) {
		   	glfwDestroyWindow(s->w);
	   	}

		ggui_quit(s->gui);
		free(s);
	}
	
	glfwTerminate();
	SDL_Quit();
}


double synth_oscillate(int waveform_type, double input, double detail) {
	double out = 0.0;
	switch(waveform_type) {
		case W_SINE:
			out = sin(input);
			break;

		case W_TRIANGLE:
			out = asin(sin(input));
			break;
		
		case W_SAW:
			for(double i = 1.0; i <= detail; i += 1.0) {
				out += sin(i*input)/i;
			}
			out *= 0.5;
			break;

		case W_REVERSE_SAW:
			for(double i = 1.0; i <= detail; i += 1.0) {
				out += sin(i*input-i)/i;
			}
			out *= 0.5;
			break;

		case W_SQUARE:
			out = SIGN(sin(input))*0.5;
			break;

		default: break;
	}
	return out;
}

double synth_osc_update(struct osc_t* osc, double time) {
	return synth_oscillate(osc->waveform, 
			sin(TO2PI(osc->tune)*time), osc->detail)*osc->volume;
}

double synth_lfo_update(struct lfo_t* osc, double time) {
	return 0.0;
}


void sdl_audio_callback(void* userdata, u8* stream, int bytes) {
	struct synth_t* synth = userdata;
	short* buf = (short*)stream;
	u32 buf_len = bytes/sizeof *buf;

	if(synth != NULL && buf != NULL) {
		//synth->sound_output = 0.0;

		for(u32 i = 0; i < buf_len; i++) {
			double out = 0.0;
			out = synth_osc_update(&synth->osc[0], synth->time);

			buf[i] = (short)(out*8000.0);

			synth->time_pos += 1.0;
			synth->time = synth->time_pos/(double)synth->audio.freq;
		}
	}
}

void glfw_mouse_button_callback(GLFWwindow* w, int button, int act, int mods) {
	struct ggui* g = glfwGetWindowUserPointer(w);

	if(act == GLFW_PRESS) {
		g->flags |= GGUI_MOUSE_DOWN;
		g->flags |= GGUI_MOUSE_HOLD_DOWN;
	}
	else if(act == GLFW_RELEASE) {
		g->flags &= ~GGUI_MOUSE_DOWN;
		g->flags &= ~GGUI_MOUSE_HOLD_DOWN;
	}

}

void glfw_cursor_position_callback(GLFWwindow* w, double x, double y) {
	struct ggui* g = glfwGetWindowUserPointer(w);
	g->mouse_x = x;
	g->mouse_y = y;
}



