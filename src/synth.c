#include <stdio.h>

#include "synth.h"

#define MIN(a, b) (a <= b ? b : a)
#define MAX(a, b) (a >= b ? b : a)
#define W(a) (a*M_PI*2.0)
#define BASEFREQ 110.0


struct __synthdata {
	
	u32 sample_rate;
	double time;
	double previous_out;
	double master_vol;
	struct osc_t o[SYNTH_NUM_OSC];

} synthdata;


double osc(u32 i) {
	u32 type = synthdata.o[i].wave_type;
	double hz = synthdata.o[i].hz;
	double res = 0.0;
	if(hz > 0.0) {
		switch(type) {
		
			case O_SINE:
				res = sin(W(hz)*synthdata.time);
				break;
			
			case O_SAW:
				res = (2.0/M_PI)*(hz*M_PI*fmod(synthdata.time,1.0/hz)-(M_PI/2.0));
				break;
			
			case O_SQUARE:
				res = sin(W(hz)*synthdata.time) > 0.0 ? 1.0 : -1.0;
				break;
			
			case O_TRIANGLE:
				res = asin(sin(W(hz)*synthdata.time))*(2.0/M_PI);
				break;

			case O_NOISE:
				res = 2.0*((double)rand()/(double)RAND_MAX)-1.0;
				break;

			default: 
				res = 0.0;
				break;
		}
	}

	return res;
}

double clip(double a, double max) {
	return (a >= 0.0) ? fmin(a, max) : fmax(a, -max);
}

void sdlaudio_callback(void* userdata, u8* stream, int bytes) {

	short* buf = (short*)stream;
	const int buflen = bytes / sizeof *buf;
	for(int i = 0; i < buflen; i++) {
		double out = 0.0;
		
		for(int i = 0; i < SYNTH_NUM_OSC; i++) {
			out += synthdata.o[i].vol * osc(i);
		}

		buf[i] = (short)(clip(out*synthdata.master_vol,1.0)*(65536/SYNTH_NUM_OSC));
		synthdata.time += (1.0/(double)synthdata.sample_rate);
	}
}


void synth_playkey(int key) {
	double hz = BASEFREQ*pow(pow(2.0,1.0/12.0),key);
	for(int i = 0; i < SYNTH_NUM_OSC; i++) {
		synthdata.o[i].hz = hz;
	}
}

void synth_set_paused(u8 b) {
	SDL_PauseAudio(b);
}

void synth_set_hz(double hz, u32 osc_num) {
	synthdata.o[osc_num].hz = hz;
}

void synth_set_osc_type(u32 type, u32 osc_num) {
	synthdata.o[osc_num].wave_type = type;
}

u32 synth_get_osc_type(u32 osc_num) {
	return synthdata.o[osc_num].wave_type;
}
/*
double synth_get_previous_out() {
	return synthdata.previous_out;
}
*/
double* synth_get_master_vol() {
	return &synthdata.master_vol;
}

struct osc_t* synth_get_osc(u32 osc_num) {
	return &synthdata.o[osc_num];
}

void synth_init() {
	SDL_AudioSpec asreq;
	SDL_AudioSpec asgot;

	asreq.format   = AUDIO_S16SYS;
	asreq.freq     = 22050*2;
	asreq.samples  = 512;
	asreq.channels = 1;
	asreq.callback = sdlaudio_callback;

	if(SDL_OpenAudio(&asreq, &asgot) < 0) {
		fprintf(stderr, "SDL_OpenAudio() failed! %s\n", SDL_GetError());
	}
	
	synthdata.time       = 0.0;
	synthdata.master_vol   = 0.5;
	synthdata.sample_rate  = asgot.freq;

	for(int i = 0; i < SYNTH_NUM_OSC; i++) {
		synthdata.o[i].wave_type = O_SQUARE;
		synthdata.o[i].hz  = 0.0;
		synthdata.o[i].vol = 0.5;
	}
	
	synth_set_paused(0);
	printf("%i\n", asgot.freq);

}

void synth_quit() {
	SDL_PauseAudio(1);
	SDL_CloseAudio();
}


