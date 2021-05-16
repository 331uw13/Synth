#ifndef SYNTH_H
#define SYNTH_H

#include <math.h>
#include <SDL2/SDL_audio.h>

#include "typedefs.h"


#define SYNTH_NUM_OSC 3
#define SYNTH_MAX_VOL 1.0

#define O_SINE     0
#define O_SAW      1
#define O_SQUARE   2
#define O_TRIANGLE 3
#define O_NOISE    4

#define MIN(a, b) (a <= b ? b : a)
#define MAX(a, b) (a >= b ? b : a)


struct osc_t {
	u32    wave_type;
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
//	double key_down_tp; // Key down timepoint.
//	double key_up_tp;   // Key up timepoint.
//	u8    is_on;
};


// TODO: remove useless stuff...


void synth_set_key_on(u32 key);
void synth_set_key_off(u32 key);
void synth_set_paused(u8 b);

struct osc_t* synth_osc(u32 osc_num);
struct env_t* synth_env(u32 env_num);

double  synth_get_previous_out();
double* synth_get_master_vol();

void synth_init();
void synth_quit();

#endif
