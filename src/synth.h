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
void oscillate(struct osc_t* osc, double time, double hz);



#endif

