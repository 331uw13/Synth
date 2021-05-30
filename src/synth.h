#ifndef SYNTH_H
#define SYNTH_H

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "things.h"
#include "gui.h"


struct state_t*  synth_init();
void             synth_quit(struct state_t* s);

double clip(double v, double max);

void synth_oscillate(int waveform, double input, double* output);
void synth_lfo_update(struct lfo_t* lfo, double time);
void synth_osc_update(struct osc_t* osc, double time, double hz);
void synth_sequencer_update(struct state_t* s);


#endif

