# Mandelbrot Visualizer
This simple C++ programm implements a configurable realtime visualizer for the classic Mandelbrot set. It makes use of an OpenGL Shader for GPU accelerated calculation of the set. Screenshots can be exported to .bmp files.

## Requirements
- CMake 2.8 or higher
- OpenGL 2.1 (GLSL version 1.20) or higher 
- SDL2 Library (https://libsdl.org)

## Building (Linux)
- create build directory: `mkdir build && cd build`
- build project: `cmake ../ && make`
- leave build directory: `cd ../`
- run with `./build/mandelbrot`

## Commandline Options
|Flag|Description|
|---|---|
|`--help`|show help|
|`--fullscreen`|sets window to fullscreen mode|
|`--framerate <fps>`|set framerate|
|`--multisamples <samples>`|specify number of samples for multisampling (e.g. 2, 4, 8)|
|`--max_iterations <value>` |number of maximum iterations to determine whether value is in the set|
|`--double_precision`|use 64 bit floats instead of 32 bit floats (requires OpenGL version 4.1 or higher)|
|`--colors <file>`|specify a .bmp file containing a colormap to define the colors used for rendering (have a look at `color_maps/blue.bmp`) |
|`--julia`|enables full julia set instead of mandelbrot (start value for z can be selected using the mouse)|
|`--nearest`|use nearest texture filtering for the color map instead of linear|
|`--location <file>`|specify a file from which a location on the fractal is loaded|

## Controls:
- Move the mouse while pressing down the left mouse button to pan
- Use the mouse wheel to zoom in/out
- Press <j> to toggle full julia set
- When julia set is activated, press the right mouse button to select an offset c in the function `f(z) = z^2 + c`
- Press <r> to reset everything
- Press <d>/<h> to double/halfen the current maximum iterations
- Press <s> to make a screen shot (saved as `mandelbrot.bmp`)
- Press <m> toggle multisampling (only available if option `--multisamples` was set)

## References
- Wikipedia: https://en.wikipedia.org/wiki/Mandelbrot_set
- Awesome Numberphile video: https://www.youtube.com/watch?v=NGMRB4O922I
