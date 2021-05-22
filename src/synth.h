#ifndef SYNTH_H
#define SYNTH_H

#include <math.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_audio.h>

#include "typedefs.h"


#define SYNTH_NUM_OSC 3
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
	double tempo;
	double sub_beats;
	double beat_time;
	double time;
	double note_time;
	void(*callback)(struct seq_t*);
	u8 enabled;
};

struct osc_t {
	int    wave_type;
	double hz;
	double vol;
	
	double note_offset;
	double lfo_freq;
	double lfo_ampl;
	//u8     enabled;
};

struct env_t {
	double attack;
	double decay;
	double release;
	double sustain;
};


// TODO: remove useless stuff...


void synth_set_key_on(u32 key);
void synth_set_key_off(u32 key);
void synth_set_paused(u8 b);

struct osc_t* synth_osc(u32 osc_num);
struct env_t* synth_env(u32 env_num);
struct seq_t* synth_seq();

double  synth_get_previous_out();
double* synth_get_master_vol();

void synth_init();
void synth_quit();

void synth_set_seq_callback(void(*callback)(struct seq_t*));

#endif
