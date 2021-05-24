#ifndef SYNTH_H
#define SYNTH_H

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "things.h"


struct state_t*  synth_init();
void             synth_quit(struct state_t* s);


void synth_play_note(u32 index);


#endif




