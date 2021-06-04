#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "synth.h"



static void glfw_mouse_button_callback(GLFWwindow* w, int button, int act, int mods);
static void glfw_cursor_position_callback(GLFWwindow* w, double x, double y);



struct state_t*  synth_init() {
	struct state_t* s = malloc(sizeof *s);
	
	if(s == NULL) {
		fprintf(stderr, "ERROR: Failed to allocate %li bytes of memory! errno: %i\n", 
				sizeof *s, errno);

		goto finish;
	}

	s->flags = 0;
	s->w = NULL;
	s->gui = NULL;

	glfwInitHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwInitHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwInitHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
	
	if(!glfwInit()) {
		fprintf(stderr, "ERROR: Failed to initialize glfw!\n");
		s->flags = NOT_INITIALIZED;
		goto finish;
	}

	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	
	if((s->w = glfwCreateWindow(1200, 850, WINDOW_TITLE, NULL, NULL)) == NULL) {
		fprintf(stderr, "ERROR: Failed to create window!\n");
		s->flags = NOT_INITIALIZED;
		goto finish;
	}

	glfwMakeContextCurrent(s->w);
	glfwSwapInterval(1);

	if(glewInit()) {
		fprintf(stderr, "ERROR: Failed to initialize glew!\n");
		s->flags = NOT_INITIALIZED;
		goto finish;
	}

	if((s->gui = ggui_init()) == NULL) {
		fprintf(stderr, "ERROR: Failed to initialize gui!\n");
		s->flags = NOT_INITIALIZED;
		goto finish;
	}

	glfwSetWindowUserPointer(s->w, s->gui);
	glfwGetWindowSize(s->w, &s->gui->win_w, &s->gui->win_h);

	glfwSetMouseButtonCallback(s->w, glfw_mouse_button_callback);
	glfwSetCursorPosCallback(s->w,   glfw_cursor_position_callback);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

finish:
	return s;
}

void synth_quit(struct state_t* s) {
	if(s != NULL) {
		if(s->w != NULL) {
		   	glfwDestroyWindow(s->w);
	   	}

		ggui_quit(s->gui);
		free(s);
	}
	
	glfwTerminate();
}


void glfw_mouse_button_callback(GLFWwindow* w, int button, int act, int mods) {
	struct ggui* g = glfwGetWindowUserPointer(w);

	if(act == GLFW_PRESS) {
		g->flags |= GGUI_MOUSE_DOWN;
		g->flags |= GGUI_MOUSE_HOLD_DOWN;
	}
	else if(act == GLFW_RELEASE) {
		g->flags &= ~GGUI_MOUSE_DOWN;
		g->flags &= ~GGUI_MOUSE_HOLD_DOWN;
	}

}

void glfw_cursor_position_callback(GLFWwindow* w, double x, double y) {
	struct ggui* g = glfwGetWindowUserPointer(w);
	g->mouse_x = x;
	g->mouse_y = y;
}



