#ifndef THINGS_H
#define THINGS_H

#include <math.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <SDL2/SDL_audio.h>

#include "ggui/ggui.h"


typedef unsigned char      u8;
typedef unsigned short     u16;
typedef unsigned int       u32;
typedef long unsigned int  u64;

#define WINDOW_TITLE  "Synthesizer! and learning how do they work."

#define SHOULD_QUIT       (1<<0)
#define NOT_INITIALIZED   (1<<1)

#define TO2PI(a) ((a)*M_PI*2.0)
#define SIGN(a) ((a > 0)-(a < 0))

#define SYNTH_NUM_OSC 3
#define SYNTH_NUM_LFO 3
#define SYNTH_NUM_ENV 4
#define SYNTH_NUM_WAVE_OPTIONS 4

#define W_SINE         0
#define W_TRIANGLE     1
#define W_SAW          2
#define W_REVERSE_SAW  3
#define W_SQUARE       4



struct osc_t {
	u8     in_use;
	double volume;
	double tone;
	double saw_detail;
	int waveform;
};

struct lfo_t {
};

struct env_t {
};

struct synth_t {
	double main_vol;
	double time;
	double time_pos;
	double sound_output;

	struct osc_t osc[SYNTH_NUM_OSC];
	struct lfo_t lfo[SYNTH_NUM_LFO];
	struct env_t env[SYNTH_NUM_ENV];
	
	SDL_AudioSpec audio;
};



struct state_t {
	int flags;
	GLFWwindow* w;

	struct synth_t synth;
	struct ggui* gui;
};



#endif
