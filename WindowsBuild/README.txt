Usage: Put optional arguments in 'args.txt' and run the application.

options:
--help                    show this help
--fullscreen              sets window to fullscreen mode
--framerate <fps>         set framerate
--multisamples <samples>  specify number of samples for multisampling (e.g. 2, 4, 8)
--max_iterations <value>  number of maximum iterations to determine whether value is in the set
--double_precision        use 64 bit floats instead of 32 bit floats (requires GLSL version >= 4.0)
--colors <file>           specify a .bmp file containing a colormap
--julia                   enables full julia set instead of mandelbrot (start value for z can be selected using the mouse)
--nearest                 use nearest texture filtering for the color map instead of linear
--location <file>         specify a file from which a location on the fractal is loaded

Controls:
Move the mouse while pressing down the left mouse button to pan.
Use the mouse wheel to zoom in/out.
Press <j> to toggle full julia set.
When julia set is activated, press the right mouse button to select an offset c in the function f(z) = z^2 + c.
Press <r> to reset everything.
Press <d>/<h> to double/halfen the current max_iterations.
Press <s> to make a screen shot (saved as 'mandelbrot.bmp').
Press <m> toggle multisampling (only available if option --multisamples set).
