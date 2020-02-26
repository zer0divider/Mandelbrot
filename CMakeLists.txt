cmake_minimum_required(VERSION 2.8)
project(mandelbrot)

find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)

set(MANDELBROT_SOURCES
	mandelbrot.cpp
	mandelbrot.h
	glew/glew.c
	mandel_shader.h
	mandel_shader.cpp
	mandel_shader_source.cpp
#	framework.cpp
#	framework.h
#	shader.cpp
#	shader.h
)

include_directories(${OPENGL_INCLUDE_DIRS})
include_directories(${SDL2_INCLUDE_DIRS})

add_executable(mandelbrot ${MANDELBROT_SOURCES})
target_link_libraries(mandelbrot ${SDL2_LIBRARIES} ${OPENGL_LIBRARIES})
