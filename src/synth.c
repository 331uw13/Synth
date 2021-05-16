#include <stdio.h>

#include "synth.h"

#define W(a) (a*M_PI*2.0)
#define BASE_FREQ 130.0



struct note_t {
	u8 playing;
	u8 used;
	double on_tp;
	double off_tp;
};

// NOTE: maybe just pass pointer to 'program state' to all functions...
struct __synth {
	u32 sample_rate;
	double time;
	double prev_out;
	double master_vol;
	struct osc_t o[SYNTH_NUM_OSC];
	struct env_t e[SYNTH_NUM_OSC];
	struct note_t notes[26];
} synth;


double oscillate(u32 i, double hz) {
	double res = 0.0;
	struct osc_t* o = &synth.o[i];
	u32 type = o->wave_type;
	double frq = W(hz)*synth.time+o->lfo_ampl*hz*sin(W(o->lfo_freq)*synth.time);

	if(hz != 0.0) {
		switch(type) {
		
			case O_SINE:
				res = sin(frq);
				break;
			
			case O_SAW:
				for(double i = 1.0; i <= 100; i++) {
					res += sin(i*frq)/i;
				}
				res *= (2.0/M_PI);
				break;
			
			case O_SQUARE:
				res = sin(frq) > 0.0 ? 1.0 : -1.0;
				break;
			
			case O_TRIANGLE:
				res = asin(sin(frq))*(2.0/M_PI);
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

double get_key_hz(int key) {
	return BASE_FREQ*pow(pow(2.0, 1.0/12.0), key);
}

double clip(double a, double max) {
	return (a >= 0.0) ? fmin(a, max) : fmax(a, -max);
}

double get_env_amp(struct env_t* e, struct note_t* note, double time, double amp) {
	double a = 0.0;

	if(note->used) {
		double elapsed = time - note->on_tp;
		
		if(elapsed <= e->attack) {
			a = (elapsed/e->attack)*amp;
		}
		if(elapsed > e->attack && elapsed <= (e->attack+e->decay)) {
			a = ((elapsed - e->attack)/e->decay)*(e->sustain-amp)+amp;
		}
		if(elapsed > (e->attack+e->decay)) {
			a = e->sustain;
		}
	}
	else {
		a = ((time - note->off_tp)/e->release)*(-e->sustain)+e->sustain;
	}

	if(a >= 1.0) {
		a = 1.0;
	}
	else if(a <= 0.0001) {
		a = 0.0;
	}

	return a;
}

void sdlaudio_callback(void* userdata, u8* stream, int bytes) {
	short* buf = (short*)stream;
	const int buflen = bytes / sizeof *buf;

	for(int i = 0; i < buflen; i++) {
		double out = 0.0;
		for(int i = 0; i < 26; i++) {
			struct note_t* note = &synth.notes[i];
			double note_amp = 0.0;
			if(note->playing) {
				for(int j = 0; j < SYNTH_NUM_OSC; j++) {
					struct osc_t* o = &synth.o[j];
					double amp = get_env_amp(&synth.e[j], note, synth.time, o->vol);
					out += (oscillate(j, get_key_hz(round(i+o->note_offset)))*amp)*o->vol;
					note_amp += amp;
				}
			}

			if(note_amp <= 0.0 && !note->used) {
				note->playing = 0;
			}
			
			synth.prev_out = out;
		}

		buf[i] = (short)(clip(out*synth.master_vol,1.0)*(65536/SYNTH_NUM_OSC));
		synth.time += 1.0/((double)synth.sample_rate);
	}
}


void synth_set_key_on(u32 key) {
	struct note_t* note = &synth.notes[key];

	if(note->off_tp >= note->on_tp)  {
		note->on_tp = synth.time;
		note->playing = 1;
		note->used = 1;
	}
}


void synth_set_key_off(u32 key) {
	struct note_t* note = &synth.notes[key];

	note->off_tp = synth.time;
	note->used = 0;
}


void synth_set_paused(u8 b) {
	SDL_PauseAudio(b);
}

double* synth_get_master_vol() {
	return &synth.master_vol;
}

double synth_get_previous_out() {
	return synth.prev_out;
}

struct osc_t* synth_osc(u32 osc_num) {
	return &synth.o[osc_num];
}

struct env_t* synth_env(u32 env_num) {
	return &synth.e[env_num];
}

void synth_init() {
	SDL_AudioSpec asreq;
	SDL_AudioSpec asgot;

	asreq.format   = AUDIO_S16SYS;
	asreq.freq     = 22050;
	asreq.samples  = 256;
	asreq.channels = 1;
	asreq.callback = sdlaudio_callback;

	if(SDL_OpenAudio(&asreq, &asgot) < 0) {
		fprintf(stderr, "SDL_OpenAudio() failed! %s\n", SDL_GetError());
	}
	
	synth.time         = 0.0;
	synth.master_vol   = 0.55;
	synth.sample_rate  = asgot.freq;

	for(int i = 0; i < 26; i++) {
		synth.notes[i].used = 0;
		synth.notes[i].on_tp = 0.0;
		synth.notes[i].off_tp = 0.0;
	}


	for(int i = 0; i < SYNTH_NUM_OSC; i++) {
		synth.o[i].wave_type = O_SQUARE;
		synth.o[i].hz  = 0.0;
		synth.o[i].vol = 0.5;
		synth.o[i].note_offset = 0.0;
		synth.o[i].lfo_freq   = 0.0;
		synth.o[i].lfo_ampl   = 0.1;
		synth.e[i].attack     = 0.01;
		synth.e[i].decay      = 0.01;
		synth.e[i].release    = 0.05;
		synth.e[i].sustain    = synth.o[i].vol;
	}
	
	synth.prev_out = 0.0;
	
	synth_set_paused(0);
	printf("%i\n", asgot.freq);

}

void synth_quit() {
	SDL_PauseAudio(1);
	SDL_CloseAudio();
}


