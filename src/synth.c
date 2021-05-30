#include <assert.h>
#include <errno.h>

#include "synth.h"



static void audio_callback(void* userdata, u8* stream, int bytes);


struct state_t* synth_init(int freq, int samples) {
	struct state_t* s = malloc(sizeof *s);

	if(s == NULL) {
		fprintf(stderr, "ERROR: Failed to allocate %li bytes of memory!\n"
				"errno: %i\n", sizeof *s, errno);
		goto finish;
	}

	printf("state_t size = %li bytes\n", sizeof *s); // DELETE LATER.


	// NOTE: create function to print SDL error?

	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
		fprintf(stderr, "ERROR: %s\n", SDL_GetError());
		s->flags = NOT_INITIALIZED;
		goto finish;
	}

	if(TTF_Init() < 0) {
		fprintf(stderr, "ERROR: %s", SDL_GetError());
		s->flags = NOT_INITIALIZED;
		goto finish;
	}

	if((s->font = TTF_OpenFont(FONT_FILE, 10)) == NULL) {
		fprintf(stderr, "ERROR: While trying to open font: %s\n", SDL_GetError());
		s->flags = NOT_INITIALIZED;
		goto finish;
	}

	if((s->w = SDL_CreateWindow(WINDOW_TITLE, 
					SDL_WINDOWPOS_CENTERED,
				   	SDL_WINDOWPOS_CENTERED, 1500, 850,
					SDL_WINDOW_SHOWN)) == NULL) {
	
		fprintf(stderr, "ERROR: Failed to create new window! %s\n", SDL_GetError());
		s->flags = NOT_INITIALIZED;
		goto finish;
	}

	if((s->r = SDL_CreateRenderer(s->w, -1, SDL_RENDERER_ACCELERATED)) == NULL) {
		fprintf(stderr, "ERROR: Failed to create renderer! %s\n", SDL_GetError());
		s->flags = NOT_INITIALIZED;
		goto finish;
	}


	SDL_AudioSpec request;
	
	request.freq = freq;
	request.format = AUDIO_S16SYS;
	request.samples = samples;
	request.callback = audio_callback;
	request.userdata = (void*)s;
	request.channels = 1;

	if(SDL_OpenAudio(&request, &s->audio) < 0) {
		fprintf(stderr, "ERROR: Failed to open audio device! %s\n", SDL_GetError());
		goto finish;
	}

	SDL_GetWindowSize(s->w, &s->window_w, &s->window_h);

	s->main_vol  = 0.2;
	s->time      = 0.0;
	s->time_pos  = 0.0;

	s->seq.tempo    = 100.0;
	s->seq.time     = 0.0;
	s->focus_index  = -1;

	create_text(s, "in",  NULL);
	create_text(s, "out", NULL);

	for(int i = 0; i < SYNTH_NUM_OSC; i++) {
		struct osc_t* osc = &s->osc[i];
		osc->waveform = W_SINE;
		osc->vol = 1.0;
		osc->hz = 130.0;
		osc->detune = 0.0;
		osc->input = 0.0;
		osc->output = NULL;
	}

	for(int i = 0; i < SYNTH_NUM_LFO; i++) {
		struct lfo_t* lfo = &s->lfo[i];
		lfo->waveform = W_SINE;
		lfo->ampl = 0.0;
		lfo->freq = 0.0;
		lfo->hz = 100.0;
		lfo->input = 0.0;
		lfo->output = NULL;
	}

	SDL_PauseAudio(0);


finish:

	return s;
}


void synth_quit(struct state_t* s) {
	
	for(u32 i = 0; i < s->gui.num_texts; i++) {
		destroy_text(&s->gui.texts[i]);
	}

	if(s->w != NULL) { SDL_DestroyWindow(s->w);    }
	if(s->r != NULL) { SDL_DestroyRenderer(s->r);  }
	SDL_Quit();
	free(s);

	puts("Bye!");
	exit(EXIT_SUCCESS);
}

double clip(double v, double max) {
	return (v > 0.0 ? MIN(v, max) : MAX(v, -max));
}

void synth_oscillate(int waveform, double input, double* output) {
	switch(waveform) {
	
		case W_SINE:
			*output = sin(input);
			break;

		case W_ABS_SINE:
			*output = fabs(sin(input-0.5));
			break;

		case W_TRIANGLE:
			*output = asin(sin(input));
			break;

		case W_SAW:
			for(double i = 1.0; i <= 100.0; i += 1.0) {
				*output += sin(i*input)/i;
			}
			*output *= 0.5;
			break;
		
		case W_SQUARE:
			*output = SIGN(sin(input))*0.5;
			break;

	
		default: break;
	}
}

void synth_lfo_update(struct lfo_t* lfo, double time) {
	if(lfo->output != NULL) {
		synth_oscillate(lfo->waveform, lfo->ampl*lfo->hz*sin(TO2PI(lfo->freq)*time+lfo->input), lfo->output);
	}
}

void synth_osc_update(struct osc_t* osc, double time, double hz) {
	if(osc->output != NULL) {
		synth_oscillate(osc->waveform, TO2PI(hz+osc->detune)*time+osc->input, osc->output);
		*osc->output *= osc->vol;
	}
}

void synth_sequencer_update(struct state_t* s) {
	if(s->seq.time >= (60.0/s->seq.tempo)/4.0) {
		s->seq.time = 0.0;
	}
}


void audio_callback(void* userdata, u8* stream, int bytes) {
	struct state_t* s = (struct state_t*)userdata;
	if(bytes <= 0 || s == NULL) { return; }

	short* buf = (short*)stream;
	const u32 buf_len = bytes/sizeof* buf;

	for(u32 i = 0; i < buf_len; i++) {

		for(int j = 0; j < SYNTH_NUM_LFO; j++) {
			synth_lfo_update(&s->lfo[j], s->time);
		}

		for(int j = 0; j < SYNTH_NUM_OSC; j++) {
			synth_osc_update(&s->osc[j], s->time, s->osc[j].hz);
		}


		buf[i] = (short)(clip(s->sound_output*s->main_vol, 1.0)*VOLUME_SCALE);

		s->sound_output = 0.0;
		s->time_pos += 1.0;
		if(s->time_pos >= 10000000.0) { s->time_pos = 0.0; }
		
		s->time = s->time_pos/s->audio.freq;
	}


}



/*
struct note_t {
	u8 playing;
	u8 used;
	double on_tp;
	double off_tp;
	double play_time;
};
*/
// NOTE: maybe just pass pointer to 'program state' to all functions...
/*
struct __synth {
	double time;
	double delta_time;
	double prev_out;
	double master_vol;
	double note_time;
	double pos;
	struct osc_t o[SYNTH_NUM_OSC];
	struct env_t e[SYNTH_NUM_ENV];
	struct lfo_t l[SYNTH_NUM_LFO];
	struct note_t notes[26];
} synth;
*/
/*
double synth_lfo_out(u32 num) {
	struct lfo_t* lfo = &synth.l[num];
	return lfo->ampl*sin(W(lfo->freq)*synth.time);
}

double oscillate(struct osc_t* o, double hz) {
	double res = 0.0;
	
	//struct lfo_t* lfo = &synth.l[0];
	u32 type = o->wave_type;
	double frq = W(hz)*synth.time;//+lfo->ampl*hz*sin(W(lfo->freq)*synth.time);

	switch(type) {
	
		case O_SINE:
			res = sin(frq);
			break;
		
		case O_SAW:
			for(double i = 1.0; i <= 50; i++) {
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

	return res;
}

double get_key_hz(int key) {
	return BASE_FREQ*pow(pow(2.0, 1.0/12.0), key);
}

double clip(double a, double max) {
	return (a >= 0.0) ? fmin(a, max) : fmax(a, -max);
}

double get_env_amp(struct env_t* e, struct note_t* note, double time, double amp) {
	double a = 1.0;
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

		double out = 0.0;
		for(int i = 0; i < 25; i++) {
			struct note_t* note = &synth.notes[i];
			double note_amp = 0.0;
			if(note->playing) {
				for(int j = 0; j < SYNTH_NUM_OSC; j++) {
					struct osc_t* o = &synth.o[j];
					double amp = get_env_amp(&synth.e[j], note, synth.time, o->vol);
					out += (oscillate(o, get_key_hz(i+o->note_offset))*amp)*o->vol;
					note_amp += amp;
				}

				if(note_amp <= 0.001 && !note->used) {
					note->playing = 0;
				}
			}
		}

		if(synth.pos != 0.0) {
			synth.time = synth.pos/(double)FREQUENCY;
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

double* synth_master_vol() {
	return &synth.master_vol;
}

double* synth_note_time() {
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
*/



