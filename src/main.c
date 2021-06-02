#include <stdio.h>

#include "synth.h"
#include "util.h"


void main_loop(struct state_t* s) {
	glClearColor(0.08, 0.08, 0.08, 1.0);


	int test = 0;

	double test_d = 0.0;

	while(!glfwWindowShouldClose(s->w) && !(s->flags & SHOULD_QUIT)) {
		glfwPollEvents();
		glClear(GL_COLOR_BUFFER_BIT);
		
		if(ggui_checkbox(s->gui, 100.0, 100.0, &test)) {
			printf("test = %i\n", test);
		}

		if(ggui_knob(s->gui, 180.0, 200.0, &test_d, 0.0, 5.0)) {
			printf("test_d = %f\n", test_d);
		}


		glfwSwapBuffers(s->w);
		s->gui->flags &= ~GGUI_MOUSE_DOWN;
	}

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




