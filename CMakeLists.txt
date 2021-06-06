cmake_minimum_required(VERSION 3.18.2)
project(Synthesizer C)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wunused-function -lglfw -lGLEW -lGL -lm -lSDL2 -lglut")
file(GLOB_RECURSE SOURCE_FILES "./src/*.c")

add_executable(
	synth
	${SOURCE_FILES}
)
