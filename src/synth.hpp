#ifndef SYNTH_H
#define SYNTH_H

#include <math.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_audio.h>
#include <GLFW/glfw3.h>

typedef unsigned char   u8;
typedef unsigned short  u16;
typedef unsigned int    u32;
typedef long unsigned int  u64;


#define SYNTH_NUM_OSC 4
#define SYNTH_NUM_ENV 4
#define SYNTH_NUM_LFO 6


#define SYNTH_MAX_VOL 1.0
#define SYNTH_SEQ_PATTERN_LENGTH 16*2

#define O_SINE     0
#define O_SAW      1
#define O_SQUARE   2
#define O_TRIANGLE 3
#define O_NOISE    4

#define MIN(a, b) (a <= b ? b : a)
#define MAX(a, b) (a >= b ? b : a)


struct seq_t {
	int pattern[SYNTH_SEQ_PATTERN_LENGTH+1];
	int current;
	int tempo;
	float time;
	u8 enabled;
};

struct rand_seq_t {
	int current;
	int tempo;
	float time;
	u8 enabled;
};

struct osc_t {
	int    wave_type;
	int   note_offset;
	float hz;
	float vol;
};

struct env_t {
	float attack;
	float decay;
	float release;
	float sustain;
};

struct lfo_t {
	float freq;
	float ampl;
};


// TODO: remove useless stuff...


void synth_set_key_on(u32 key);
void synth_set_paused(u8 b);

double synth_get_lfo(u32 num);

struct osc_t* synth_osc(u32 num);
struct env_t* synth_env(u32 num);
struct lfo_t* synth_lfo(u32 num);

float* synth_master_vol();
float* synth_note_time();
void synth_init();
void synth_quit();

#endif
