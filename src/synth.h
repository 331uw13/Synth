#ifndef SYNTH_H
#define SYNTH_H

#define SYNTH_DEFSAMPLERATE 44100
#define SYNTH_AUDIOBUFFER_SIZE 256


#include <math.h>
#include <SDL2/SDL_audio.h>

typedef unsigned int   u32;
typedef unsigned short u16;
typedef unsigned char  u8;


#define SYNTH_NUM_OSC 3
#define SYNTH_MAX_VOL 0.9

#define O_SINE     0
#define O_SAW      1
#define O_SQUARE   2
#define O_TRIANGLE 3
#define O_NOISE    4

struct osc_t {
	u32    wave_type;
	double hz;
	double vol;
};


void synth_playkey(int key);
void synth_set_paused(u8 b);

void synth_set_hz(double hz, u32 osc_num);
void synth_set_osc_type(u32 type, u32 osc_num);
u32 synth_get_osc_type(u32 osc_num);
//double synth_get_previous_out();
double* synth_get_master_vol();
struct osc_t* synth_get_osc(u32 osc_num);

void synth_init();
void synth_quit();

#endif
