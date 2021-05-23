#include <stdio.h>

#include "synth.hpp"

#define W(a) (a*M_PI*2.0)
#define BASE_FREQ 130.0

#define FREQUENCY 44100
#define SAMPLES   512



struct note_t {
	u8 playing;
	u8 used;
	float on_tp;
	float off_tp;
	float play_time;
};

// NOTE: maybe just pass pointer to 'program state' to all functions...
struct __synth {
	float time;
	float delta_time;
	float prev_out;
	float master_vol;
	float note_time;
	float pos;
	struct osc_t o[SYNTH_NUM_OSC];
	struct env_t e[SYNTH_NUM_ENV];
	struct lfo_t l[SYNTH_NUM_LFO];
	struct note_t notes[26];
} synth;

double synth_get_lfo(u32 num) {
	struct lfo_t* lfo = &synth.l[num];
	return lfo->ampl*sin(W(lfo->freq)*synth.time);
}

float oscillate(struct osc_t* o, float hz) {
	float res = 0.0;
	
	//struct lfo_t* lfo = &synth.l[0];
	u32 type = o->wave_type;
	float frq = W(hz)*synth.time;//+lfo->ampl*hz*sin(W(lfo->freq)*synth.time);

	switch(type) {
	
		case O_SINE:
			res = sin(frq);
			break;
		
		case O_SAW:
			for(float i = 1.0; i <= 50; i++) {
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
			res = 2.0*((float)rand()/(float)RAND_MAX)-1.0;
			break;

		default: 
			res = 0.0;
			break;
	}

	return res;
}

float get_key_hz(int key) {
	return BASE_FREQ*pow(pow(2.0, 1.0/12.0), key);
}

float clip(float a, float max) {
	return (a >= 0.0) ? fmin(a, max) : fmax(a, -max);
}

float get_env_amp(struct env_t* e, struct note_t* note, float time, float amp) {
	float a = 1.0;
	if(note->used) {
		float elapsed = time - note->on_tp;
		
		if(elapsed <= e->attack) {
			a = (elapsed/e->attack)*amp;
		}
		if(elapsed > e->attack && elapsed <= (e->attack+e->decay)) {
			a = ((elapsed - e->attack)/e->decay)*(e->sustain-amp)+amp;
		}
		if(elapsed > (e->attack+e->decay)) {
			a = e->sustain;
		}

		if(elapsed > synth.note_time) {
			note->off_tp = synth.time;
			note->used = 0;
		}

	}
	else {
		a = ((time - note->off_tp)/e->release)*(-e->sustain)+e->sustain;
	}

	if(a >= 1.0) {
		a = 1.0;
	}
	else if(a <= 0.001) {
		a = 0.0;
	}

	return a;
}

void sdlaudio_callback(void* userdata, u8* stream, int bytes) {
	short* buf = (short*)stream;
	const int buflen = bytes / sizeof *buf;
	

	for(int i = 0; i < buflen; i++) {
		
		synth.pos += 1.0;

		if(synth.pos >= 10000000.0) {
			synth.pos = 0.0;
		}

		float out = 0.0;
		for(int i = 0; i < 25; i++) {
			struct note_t* note = &synth.notes[i];
			float note_amp = 0.0;
			if(note->playing) {
				for(int j = 0; j < SYNTH_NUM_OSC; j++) {
					struct osc_t* o = &synth.o[j];
					float amp = get_env_amp(&synth.e[j], note, synth.time, o->vol);
					out += (oscillate(o, get_key_hz(i+o->note_offset))*amp)*o->vol;
					note_amp += amp;
				}

				if(note_amp <= 0.001 && !note->used) {
					note->playing = 0;
				}
			}
		}

		if(synth.pos != 0.0) {
			synth.time = synth.pos/(float)FREQUENCY;
		}
		

		buf[i] = (short)(clip(out*synth.master_vol,1.0)*(65536/SYNTH_NUM_OSC));
	}

}


void synth_set_key_on(u32 key) {
	struct note_t* note = &synth.notes[key];
	
	note->on_tp = synth.time;
	note->playing = 1;
	note->used = 1;
}


void synth_set_key_off(u32 key) {
	struct note_t* note = &synth.notes[key];
	note->off_tp = synth.time;
	note->used = 0;
}


void synth_set_paused(u8 b) {
	SDL_PauseAudio(b);
}

float* synth_master_vol() {
	return &synth.master_vol;
}

float* synth_note_time() {
	return &synth.note_time;
}

struct osc_t* synth_osc(u32 num) {
	return &synth.o[num];
}

struct env_t* synth_env(u32 num) {
	return &synth.e[num];
}

struct lfo_t* synth_lfo(u32 num) {
	return &synth.l[num];
}

void synth_init() {
	SDL_AudioSpec asreq;
	SDL_AudioSpec asgot;

	asreq.format   = AUDIO_S16SYS;
	asreq.freq     = FREQUENCY;
	asreq.samples  = SAMPLES;
	asreq.channels = 1;
	//asreq.silence  = 5;
	asreq.callback = sdlaudio_callback;

	if(SDL_OpenAudio(&asreq, &asgot) < 0) {
		fprintf(stderr, "SDL_OpenAudio() failed! %s\n", SDL_GetError());
	}
	

	synth.pos          = 0.0;
	synth.time         = 0.0;
	synth.master_vol   = 0.20;

	for(int i = 0; i < 26; i++) {
		synth.notes[i].used = 0;
		synth.notes[i].on_tp = 0.0;
		synth.notes[i].off_tp = 0.0;
	}


	for(int i = 0; i < SYNTH_NUM_OSC; i++) {
		synth.o[i].wave_type = O_SQUARE;
		synth.o[i].hz  = 0.0;
		synth.o[i].vol = 0.20;
		synth.o[i].note_offset = 0.0;
		synth.e[i].attack     = 0.001;
		synth.e[i].decay      = 0.001;
		synth.e[i].release    = 0.001;
		synth.e[i].sustain    = 1.0;
	}

	synth.note_time = 0.1;
	
	synth.prev_out = 0.0;
	synth_set_paused(0);
}

void synth_quit() {
	SDL_PauseAudio(1);
	SDL_CloseAudio();
}


