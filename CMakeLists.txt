cmake_minimum_required(VERSION 3.18.2)
project(Synthesizer C)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wunused-function -lSDL2 -lm")

file(GLOB_RECURSE SOURCE_FILES "./src/*.c")

add_executable(
	synth
	${SOURCE_FILES}
)
