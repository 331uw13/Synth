#include <string.h>

#include "termg.h"
#include "synth.h"


#define MAX_VALUES 10
#define MAX_REGIONS 32


#define CLAMP(a, min, max) (a <= min ? min : a >= max ? max : a)
#define IN_RANGE(a, min, max) (a >= min && a <= max)
#define CTRL(k) (k&0x1F)


#define TYPE_NONE    0
#define TYPE_INT     1
#define TYPE_DOUBLE  2

struct any_t {
	int type;
	union {
		double* d;
		int* i;
	};
	union {
		double max_d;
		int    max_i;
	};
	union {
		double min_d;
		int    min_i;
	};
};

struct value_t {
	char* text;
	struct any_t p;
};

struct region_t {
	struct value_t values[MAX_VALUES];
	u16 num_values;
	u8 folded;
	char* title;
};

struct cursor_t {
	u16 row;
	int region_idx;
	int value_idx;
};

struct gdata {
	struct cursor_t cur;
	struct region_t regions[MAX_REGIONS];
	u16 num_regions;
	u16 num_rows;
	u16 win_w;
	u16 win_h;

};


void add_value_double(struct gdata* g, char* text, double* ptr, double min, double max) {
	struct region_t* reg = &g->regions[g->num_regions];
	if(reg->num_values < MAX_VALUES) {
		struct value_t* v = &reg->values[reg->num_values];
		v->text = strdup(text);

		v->p.max_d = max;
		v->p.min_d = min;
		v->p.d = ptr;
		v->p.type = TYPE_DOUBLE;

		reg->num_values++;
		g->num_rows++;
	}
}

void add_value_int(struct gdata* g, char* text, int* ptr, int min, int max) {
	struct region_t* reg = &g->regions[g->num_regions];
	if(reg->num_values < MAX_VALUES) {
		struct value_t* v = &reg->values[reg->num_values];
		v->text = strdup(text);

		v->p.max_i = max;
		v->p.min_i = min;
		v->p.d = ptr;
		v->p.type = TYPE_INT;

		reg->num_values++;
		g->num_rows++;
	}
}

void begin_region(struct gdata* g, char* name) {
	if(g->num_regions < MAX_REGIONS) {
		struct region_t* reg = &g->regions[g->num_regions];
		reg->num_values = 0;
		reg->title = strdup(name);
		
		g->num_rows++;
	}
}

void end_region(struct gdata* g) {
	if(g->num_regions < MAX_REGIONS) {
		g->num_regions++;
	}
}


void draw_update(struct gdata* g) {
	u16 y = 0;

	g->cur.value_idx = -1;
	g->cur.region_idx = -1;
	for(u16 r = 0; r < g->num_regions; r++) {
		struct region_t* reg = &g->regions[r];
		const u8 current_reg = (y == g->cur.row);

		termg_color(TERMG_BG, TERMG_DARK);
		termg_color(TERMG_FG, current_reg ? TERMG_CYAN : TERMG_ORANGE);
		termg_clear_line(y);
		termg_print(2, y, "++ %s", reg->title);

		y++;

		for(u16 i = 0; i < reg->num_values; i++) {
			struct value_t* v = &reg->values[i];
			if(v->p.type != TYPE_NONE) {
				const u8 current = (y == g->cur.row);
				if(current) {
					g->cur.value_idx = i;
					g->cur.region_idx = r;
				}

				termg_color(TERMG_BG, TERMG_DARK);
				termg_color(TERMG_FG, current ? TERMG_YELLOW : TERMG_GRAY);
				termg_clear_line(y);
				if(v->p.type == TYPE_INT) {
					termg_print(4, y, "|- %10s (%i) %c", v->text, *v->p.i, current ? '<' : 0x0);
				}
				else if(v->p.type == TYPE_DOUBLE) {
					termg_print(4, y, "|- %10s (%1.2f) %c", v->text, *v->p.d, current ? '<' : 0x0);
				}
				y++;
			}
		}
	}
}


int main() {

	if(SDL_Init(SDL_INIT_AUDIO) < 0) {
		fprintf(stderr, "ERROR: %s\n", SDL_GetError());
		return -1;
	}

	termg_init();
	synth_init();
	u8 keep = 1;
	u8 first = 0;
	struct gdata g;
	
	g.num_rows = 0;
	g.num_regions = 0;
	g.win_w = 0;
	g.win_h = 0;
	g.cur.row = 0;

	g.cur.region_idx = 0;
	g.cur.value_idx = 0;

	termg_win_size(&g.win_w, &g.win_h);


	// Add settings.

	begin_region(&g, "General");
	add_value_double(&g, "Master volume", synth_get_master_vol(), 0.0, 1.0);	
	add_value_double(&g, "Note time", &synth_seq()->note_time, 0.0, 1.0);	
	add_value_double(&g, "Speed", &synth_seq()->sub_beats, 0.0, 50.0);	
	add_value_double(&g, "Tempo", &synth_seq()->tempo, 0.0, 200.0);	
	end_region(&g);


	for(int i = 0; i < SYNTH_NUM_OSC; i++) {
		char buf[32];
		sprintf(buf, "Oscillator (%i)", i);
		begin_region(&g, buf);
		struct osc_t* o = synth_osc(i);
		add_value_double(&g, "Volume", &o->vol, 0.0, 1.0);
		add_value_int(&g, "Shape", &o->wave_type, 0, 4);
		end_region(&g);
	
		memset(buf, 0, 31);
		sprintf(buf, "Envelope (%i)", i);
		begin_region(&g, buf);
		struct env_t* e = synth_env(i);
		add_value_double(&g, "Attack", &e->attack, 0.0, 1.0);
		add_value_double(&g, "Decay", &e->decay, 0.0, 1.0);
		add_value_double(&g, "Sustain", &e->sustain, 0.0, 1.0);
		add_value_double(&g, "Release", &e->release, 0.0, 1.0);
		end_region(&g);

	}


	termg_color(TERMG_BG, TERMG_DARK);
	termg_clear();


	while(keep) {
	
		struct region_t* cur_region = NULL;
		struct value_t* cur_value = NULL;
		
		draw_update(&g);


		if(IN_RANGE(g.cur.region_idx, 0, MAX_REGIONS)) {
			cur_region = &g.regions[g.cur.region_idx];
			if(cur_region != NULL && IN_RANGE(g.cur.value_idx, 0, MAX_VALUES)) {
				cur_value = &cur_region->values[g.cur.value_idx];
			}
		}


		const u8 input = getchar();
		switch(input) {

			case CTRL('q'):
				keep = 0;
				break;

			case 0xD:  // Enter
				break;

			case '1':
				synth_set_key_on(1);
				break;

			case '2':
				synth_set_key_off(1);
				break;

			case 'w':   // Up
				if(g.cur.row > 0) {
					g.cur.row--;
				}
				break;
			
			case 's':   // Down
				g.cur.row++;
				break;

			case ',':
				if(cur_value != NULL) {
					if(cur_value->p.type == TYPE_DOUBLE) {
						*cur_value->p.d = CLAMP(*cur_value->p.d-0.01, 
								cur_value->p.min_d, cur_value->p.max_d);
					}
					else if(cur_value->p.type == TYPE_INT) {
						*cur_value->p.i = CLAMP(*cur_value->p.i-1, 
								cur_value->p.min_i, cur_value->p.max_i);
					}
				}
				break;
			
			case '.':
				if(cur_value != NULL) {
					if(cur_value->p.type == TYPE_DOUBLE) {
						*cur_value->p.d = CLAMP(*cur_value->p.d+0.01, 
								cur_value->p.min_d, cur_value->p.max_d);
					}
					else if(cur_value->p.type == TYPE_INT) {
						*cur_value->p.i = CLAMP(*cur_value->p.i+1, 
								cur_value->p.min_i, cur_value->p.max_i);
					}
				}

				break;


			default:
			   	break;
		}
	

		first = 0;
	
	}

	puts("\nexit.");
	synth_quit();
	termg_quit();
	return 0;
}

