#ifndef SYNTH_H
#define SYNTH_H

#include <math.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_audio.h>

typedef unsigned char   u8;
typedef unsigned short  u16;
typedef unsigned int    u32;
typedef long unsigned int  u64;


#define SYNTH_NUM_OSC 2
#define SYNTH_NUM_ENV 2
#define SYNTH_NUM_LFO 4


#define SYNTH_MAX_VOL 1.0
#define SYNTH_SEQ_PATTERN_LENGTH 16*2

#define O_SINE     0
#define O_SAW      1
#define O_SQUARE   2
#define O_TRIANGLE 3
#define O_NOISE    4

#define MIN(a, b) (a <= b ? b : a)
#define MAX(a, b) (a >= b ? b : a)


struct seq_t {
	int pattern[SYNTH_SEQ_PATTERN_LENGTH+1];
	int current;
	int tempo;
	float beat_time;
	float time;
	float note_time;
	void(*callback)(struct seq_t*);
	u8 enabled;
};

struct osc_t {
	int    wave_type;
	float note_offset;
	float hz;
	float vol;
};

struct env_t {
	float attack;
	float decay;
	float release;
	float sustain;
};

struct lfo_t {
	float freq;
	float ampl;
};


// TODO: remove useless stuff...


void synth_set_key_on(u32 key);
void synth_set_key_off(u32 key);
void synth_set_paused(u8 b);

double synth_get_lfo();

struct osc_t* synth_osc(u32 num);
struct env_t* synth_env(u32 num);
struct lfo_t* synth_lfo(u32 num);
struct seq_t* synth_seq();

float* synth_master_vol();
void synth_init();
void synth_quit();

void synth_set_seq_callback(void(*callback)(struct seq_t*));

#endif
