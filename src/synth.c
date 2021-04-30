#include <stdio.h>

#include "synth.h"


struct __synthdata {
	u16 sample_rate;
	void(*callback)(double, double*);
} synthdata;

static void sdlaudio_callback(void* userdata, u8* rawbuffer, int bytes);


void synth_init(u16 freq, u16 buffersize, u8 num_channels) {
	synthdata.sample_rate = SYNTH_DEFSAMPLERATE;
	SDL_AudioSpec asreq;
	SDL_AudioSpec asgot;

	asreq.format = AUDIO_S16SYS;
	asreq.freq = freq;
	asreq.samples = buffersize;
	asreq.channels = num_channels;
	asreq.callback = sdlaudio_callback;

	if(SDL_OpenAudio(&asreq, &asgot) < 0) {
		fprintf(stderr, "SDL_OpenAudio() failed! %s\n", SDL_GetError());
	}

	SDL_PauseAudio(0);
}

void synth_callback(void(*callback)(double, double*)) {
	synthdata.callback = callback;
}

void synth_quit() {
	SDL_PauseAudio(1);
	SDL_CloseAudio();
}

void sdlaudio_callback(void* userdata, u8* rawbuffer, int bytes) {
	if(synthdata.callback == NULL) { return; }
	u16* buf = (u16*)rawbuffer;
	const int buflen = bytes / sizeof *buf;
	for(int i = 0; i < buflen; i++) {
		const double t = (double)i / (double)synthdata.sample_rate;
		double out = 0.0;
		synthdata.callback(t, &out);
		buf[i] = (u16)out;
	}
}



