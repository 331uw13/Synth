#ifndef SYNTH_H
#define SYNTH_H

#include "things.h"


#define SYNTH_NUM_OSC 5
#define SYNTH_NUM_LFO 3
#define SYNTH_NUM_ENV 4
#define SYNTH_NUM_WAVE_OPTIONS 4

#define W_SINE      0
#define W_TRIANGLE  1
#define W_SAW       2
#define W_SQUARE    3



struct osc_t {
	double tune;
	int waveform;
};

struct lfo_t {
};

struct env_t {
};


struct synth_t {

	double main_vol;

	struct osc_t osc[SYNTH_NUM_OSC];
	struct lfo_t osc[SYNTH_NUM_LFO];
	struct env_t osc[SYNTH_NUM_ENV];



};


struct state_t*  synth_init();
void             synth_quit(struct state_t* s);


#endif
