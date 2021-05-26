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
#define WINDOW_TITLE "Synthesizer and learning about music..."


#define SYNTH_MAX_VOL 1.0
#define SYNTH_NUM_OSC 3
#define SYNTH_NUM_ENV 3
#define SYNTH_NUM_LFO 2

#define KNOB_RADIUS 25.0
#define SYNTH_SEQ_PATTERN_LENGTH 32

#define GUI_ITEM_X_SPACE  60
#define GUI_ITEM_Y_SPACE  50
#define GUI_ITEM_X_OFFSET (KNOB_RADIUS*2+30)
#define GUI_ITEM_Y_OFFSET (KNOB_RADIUS*2+50)

#define W_SINE      0
#define W_ABS_SINE  1
#define W_SQUARE    2
#define W_TRIANGLE  3
#define W_SAW       4
#define NUM_WAVE_OPTIONS 4

#define WIRE_TYPE_NONE    -1
#define WIRE_TYPE_INPUT   (1<<0)
#define WIRE_TYPE_OUTPUT  (1<<1)

#define DATA_TYPE_INT 0
#define DATA_TYPE_DOUBLE 1
#define VOLUME_SCALE (65536/SYNTH_NUM_OSC)

#define MIN(a, b) (a <= b ? a : b)
#define MAX(a, b) (a >= b ? a : b)
#define SIGN(a) ((a > 0.0)-(a < 0.0))
#define AREA_OVERLAP(xa,ya,xb,yb,w,h) ((xa >= xb && xa <= xb+w) && (ya >= yb && ya <= yb+h))
#define TO2PI(a) ((a)*M_PI*2.0)

#define NOT_INITIALIZED (1<<0)
#define SHOULD_QUIT     (1<<1)
#define MOUSE_LEFT_DOWN   (1<<2)
#define MOUSE_RIGHT_DOWN  (1<<3)
#define DRAG_WIRE       (1<<4)



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
	double pitch;
	double vol;
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
	double freq;
	double ampl;
};

struct text_t {
	SDL_Texture* texture;
	SDL_Rect rect;
};

struct wire_point_t {
	int type;
	double* in_ptr;
	double* out_ptr;
	int color;
	int x;
	int y;
};


struct knob_t {
	struct text_t* text;
	struct wire_point_t wire_point;

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
	
	int color;
	int data_type;
	int x;
	int y;
};


struct state_t {
	SDL_Window*    w;
	SDL_Renderer*  r;
	SDL_AudioSpec  audio;
	TTF_Font*      font;
	SDL_Rect       boxes[32];

	int flags;
	int mouse_x;
	int mouse_y;
	int mouse_down_x;
	int mouse_down_y;
	int focus_index;

	double test_value; // DELETE LATER.

	double time;
	double time_pos;
	double main_vol;
	double sound_output;

	struct sequencer_t seq;
	
	struct osc_t osc[SYNTH_NUM_OSC];
	struct env_t env[SYNTH_NUM_ENV];
	struct lfo_t lfo[SYNTH_NUM_LFO];

	struct text_t texts[32];
	struct knob_t knobs[32];
	struct wire_point_t output_points[32];

	struct wire_point_t* wire_point_drag_origin;

	int num_texts;
	int num_knobs;
	int num_boxes;
	int num_output_points;
};



#endif
