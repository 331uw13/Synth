#ifndef THINGS_H
#define THINGS_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_audio.h>

typedef unsigned char      u8;
typedef unsigned short     u16;
typedef unsigned int       u32;
typedef long unsigned int  u64;

#define WINDOW_TITLE "Synthesizer and learning about music..."

#define SYNTH_MAX_VOL 1.0
#define SYNTH_NUM_OSC 3
#define SYNTH_NUM_ENV 3
#define SYNTH_NUM_LFO 2

#define KNOB_RADIUS 25.0
#define SYNTH_SEQ_PATTERN_LENGTH 32

#define O_SINE     0
#define O_SAW      1
#define O_SQUARE   2
#define O_TRIANGLE 3
#define O_NOISE    4

#define MIN(a, b) (a <= b ? a : b)
#define MAX(a, b) (a >= b ? a : b)
#define AREA_OVERLAP(xa,ya,xb,yb,w,h) ((xa >= xb && xa <= xb+w) && (ya >= yb && ya <= yb+h))


#define NOT_INITIALIZED (1<<0)
#define SHOULD_QUIT     (1<<1)
#define MOUSE_DOWN      (1<<2)
#define RESTORE_MOUSE_POS (1<<3)
//#define RENDER_UPDATE   (1<<2)




struct sequencer_t {
	int pattern[SYNTH_SEQ_PATTERN_LENGTH+1];
	int now;
	double tempo;
	double time;
	double note_time;
	u8 playing;
};

struct rand_seq_t {
	int now;
	double tempo;
	double time;
	double note_time;
	double min;
	double max;
	u8 playing;
};

struct osc_t {
	int    shape;
	double pitch;
	double vol;
};

struct env_t {
	double attack;
	double decay;
	double release;
	double sustain;
};

struct lfo_t {
	double freq;
	double ampl;
};


struct knob_t {
	double* ptr;
	double min;
	double max;
	u32 color;

	int center_x;
	int center_y;
};


struct state_t {
	SDL_Window*    w;
	SDL_Renderer*  r;
	SDL_AudioSpec  audio;

	int flags;
	int mouse_x;
	int mouse_y;
	int mouse_down_x;
	int mouse_down_y;
	int focus_index;

	int next_knob_x;
	int next_knob_y;

	double time;
	double time_pos;
	double master_vol;
	
	struct sequencer_t seq;

	struct osc_t osc[SYNTH_NUM_OSC];
	struct env_t env[SYNTH_NUM_ENV];
	struct lfo_t lfo[SYNTH_NUM_LFO];

	struct knob_t knobs[32];
	int num_knobs;

};



#endif
