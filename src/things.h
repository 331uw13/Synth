#ifndef THINGS_H
#define THINGS_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_audio.h>
#include <SDL2/SDL_ttf.h>

typedef unsigned char      u8;
typedef unsigned short     u16;
typedef unsigned int       u32;
typedef long unsigned int  u64;

#define FONT_FILE    "Topaz-8.ttf"
#define WINDOW_TITLE "Synthesizer! and learning about music..."


#define MAX_NUM_BOXES  15
#define MAX_NUM_TEXTS  32
#define MAX_NUM_KNOBS  32
#define MAX_NUM_OUTPUTS 32
#define MAX_NUM_INPUTS  32

#define SYNTH_MAX_VOL 1.0
#define SYNTH_NUM_OSC 4
#define SYNTH_NUM_ENV 3
#define SYNTH_NUM_LFO 2

#define KNOB_RADIUS 24.0
#define SYNTH_SEQ_PATTERN_LENGTH 16

#define WIREPT_RESERVED_COLOR  0x30FF30
#define WIREPT_FREE_COLOR      0x707070

#define GUI_ITEM_OFFSET  25
#define GUI_FRAME_OFFSET 3

// NOTE: rename to GUI_CELL_SIZE ?
#define GUI_CELL_HEIGHT  (KNOB_RADIUS*2+GUI_ITEM_OFFSET)  
#define GUI_CELL_WIDTH   (KNOB_RADIUS*2+GUI_ITEM_OFFSET)

// 
#define W_SINE      0
#define W_ABS_SINE  1
#define W_TRIANGLE  2
#define W_SAW       3
#define W_SQUARE    4
#define NUM_WAVE_OPTIONS 4

#define WIRE_TYPE_INPUT   0
#define WIRE_TYPE_OUTPUT  1
#define WIRE_TYPE_NONE    2

#define DATA_TYPE_INT 0
#define DATA_TYPE_DOUBLE 1
#define VOLUME_SCALE (65536/SYNTH_NUM_OSC)

// Some stuff i dont feel like writing again and again...
#define MIN(a, b) (a <= b ? a : b)
#define MAX(a, b) (a >= b ? a : b)
#define SIGN(a) ((a > 0.0)-(a < 0.0))
#define AREA_OVERLAP(xa, ya, xb, yb, w, h) ((xa >= xb && xa <= xb+w) && (ya >= yb && ya <= yb+h))
#define TO2PI(a) ((a)*M_PI*2.0)


// Flags for 'struct state_t'
#define NOT_INITIALIZED   (1<<0)
#define SHOULD_QUIT       (1<<1)
#define MOUSE_LEFT_DOWN   (1<<2)
#define MOUSE_RIGHT_DOWN  (1<<3)
#define DRAG_WIRE         (1<<4)  // This is set if user clicked wire point.


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
	double vol;
	double hz;
	double detune;
	int    waveform;

	double  input;
	double* output;
};

struct env_t {
	double attack;
	double decay;
	double release;
	double sustain;
};

struct lfo_t {
	double ampl;
	double freq;
	double hz;
	int    waveform;

	double  input;
	double* output;
};

struct text_t {
	SDL_Texture* texture;
	SDL_Rect rect;
};

struct wirept_t {
	union {
		double* in_ptr;
		double** out_ptr;
	};

	struct wirept_t* link;
	u8  reserved;
	int type;
	int color;
	int x;
	int y;
};

struct knob_t {
	union {
		struct {
			double* ptr_d;
			double min_d;
			double max_d;
		};
		struct {
			int* ptr_i;
			int min_i;
			int max_i;
		};
	};
	struct text_t* text;
	int data_type;
	int color;
	int x;
	int y;
};


struct state_t {
	SDL_Window*    w;
	SDL_Renderer*  r;
	SDL_AudioSpec  audio;
	TTF_Font*      font;

	int flags;
	int mouse_x;
	int mouse_y;
	int mouse_down_x;
	int mouse_down_y;
	int focus_index;
	int window_w;
	int window_h;

	double test_value; // DELETE LATER.
	double test_value2; // DELETE LATER.

	double time;
	double time_pos;
	double main_vol;
	double sound_output;  // For example if you set output from oscillator to point here you can probably hear it.

	struct sequencer_t seq;
	
	struct osc_t osc[SYNTH_NUM_OSC];
	struct env_t env[SYNTH_NUM_ENV];
	struct lfo_t lfo[SYNTH_NUM_LFO];


	struct {
		
		SDL_Rect          boxes   [MAX_NUM_BOXES];
		struct text_t     texts   [MAX_NUM_TEXTS];
		struct knob_t     knobs   [MAX_NUM_KNOBS];
		struct wirept_t   outputs [MAX_NUM_OUTPUTS];
		struct wirept_t   inputs  [MAX_NUM_INPUTS];
		
		u32 num_boxes;
		u32 num_texts;
		u32 num_knobs;
		u32 num_outputs;
		u32 num_inputs;

		struct wirept_t*  wirept_drag;

	} gui;
};


#endif











