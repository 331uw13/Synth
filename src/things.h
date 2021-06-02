#ifndef THINGS_H
#define THINGS_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GL/glu.h>

#include "ggui/ggui.h"


typedef unsigned char      u8;
typedef unsigned short     u16;
typedef unsigned int       u32;
typedef long unsigned int  u64;

#define WINDOW_TITLE  "Synthesizer! and learning how do they work."

#define SHOULD_QUIT       (1<<0)
#define NOT_INITIALIZED   (1<<1)


struct state_t {
	int flags;
	GLFWwindow* w;

	struct ggui* gui;
};



#endif
