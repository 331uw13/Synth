#include <stdio.h>
#include <unistd.h>

#include "synth.h"
#include "util.h"


#define OSC_OFFSET 70.0


void main_loop(struct state_t* s) {
	glClearColor(0.24, 0.14, 0.20, 1.0);

	while(!glfwWindowShouldClose(s->w) && !(s->flags & SHOULD_QUIT)) {
		glClear(GL_COLOR_BUFFER_BIT);

		ggui_text(s->gui, 100, 40, 120, "Oscillators");
		ggui_text(s->gui, 125, 80, 40, "vol");
		ggui_text(s->gui, 190, 80, 45, "wave");
		ggui_text(s->gui, 260, 80, 45, "tone");
		ggui_text(s->gui, 330, 80, 60, "sawdtl");
	
		for(int i = 0; i < SYNTH_NUM_OSC; i++) {
			struct osc_t* osc = &s->synth.osc[i];
			const double off = 120.0+(i*OSC_OFFSET);

			ggui_checkbox(s->gui, 60.0, off, &osc->in_use);
			ggui_knob_d(s->gui, 120.0, off, &osc->volume,   0.0, 1.0);
			ggui_knob_i(s->gui, 190.0, off, &osc->waveform, 0, SYNTH_NUM_WAVE_OPTIONS);
			ggui_knob_d(s->gui, 260.0, off, &osc->tone,       100.0, 400.0);
			ggui_knob_d(s->gui, 330.0, off, &osc->saw_detail, 10.0, 100.0);
		}


		if(ggui_button(s->gui, 400.0, 400.0, 0)) {
			printf("clicked!\n");
		}


		glfwSwapBuffers(s->w);
		
		s->gui->flags &= ~GGUI_MOUSE_DOWN;
		glfwWaitEvents();
	}

	puts("exit.");
}


int main() {
	struct state_t* s = NULL;
	s = synth_init();

	if(s != NULL && !(s->flags & NOT_INITIALIZED)) {
		main_loop(s);
	}

	synth_quit(s);
	return 0;
}

