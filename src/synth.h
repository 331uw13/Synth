#ifndef SYNTH_H
#define SYNTH_H

#define SYNTH_DEFSAMPLERATE 44100
#define SYNTH_AUDIOBUFFER_SIZE 512


#include <math.h>
#include <SDL2/SDL_audio.h>

typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;

void synth_init(u16 freq, u16 buffersize, u8 num_channels);
void synth_quit();

void synth_callback(void(*callback)(double, double*));


#endif
