#ifndef SYNTH_H
#define SYNTH_H

#include "things.h"


double synth_oscillate(int waveform_type, double input, double detail);

double synth_osc_update(struct osc_t* osc, double time);
double synth_lfo_update(struct lfo_t* osc, double time);



struct state_t*  synth_init();
void             synth_quit(struct state_t* s);


#endif
