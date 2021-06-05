#include <stdio.h>

#include "synth.h"
#include "util.h"




void main_loop(struct state_t* s) {
	glClearColor(0.08, 0.08, 0.08, 1.0);


	int test = 0;
	double test_d = 0.0;

	s->synth.osc[0].waveform = W_SINE;

	while(!glfwWindowShouldClose(s->w) && !(s->flags & SHOULD_QUIT)) {
		s->gui->flags &= ~GGUI_MOUSE_DOWN;
		
		glfwWaitEvents();		
		glClear(GL_COLOR_BUFFER_BIT);

		//ggui_checkbox(s->gui, 100.0, 100.0, &test);
		
		ggui_knob(s->gui, 180.0, 200.0, &s->synth.osc[0].volume, 0.0, 1.0);
		ggui_knob(s->gui, 180.0, 280.0, &s->synth.osc[0].tune, 100.0, 400.0);


		glfwSwapBuffers(s->w);
	
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

