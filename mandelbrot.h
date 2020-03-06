#include <SDL2/SDL.h>
#include "glew/glew.h"
#include "mandel_shader.h"
//#include "shader.h"
#include <cstring>
#include <cstdlib>
#include <ctype.h>

#define MANDELBROT_MAX_COLORS 1024
#define MANDELBROT_INITIAL_ZOOM 1.2
#define MANDELBROT_INITIAL_X_OFFSET -0.5
struct MandelbrotSettings{
	MandelbrotSettings(){setToDefault();}
	void setToDefault(){
		fullscreen = false;
		fps = 60;
		multisamples = 0;
		maxIterations = 128;
		julia = false;
		colors[0] = 0x000000;
		colors[1] = 0xFFFFFF;
		numColors = 2;
		doublePrecision = false;
		nearest = false;
	}
	
	bool julia;
	bool doublePrecision;
	bool nearest;
	int maxIterations;
	Uint32 colors[MANDELBROT_MAX_COLORS];
	int numColors;
	bool fullscreen;
	int fps;
	int multisamples;

	void print(){
		printf(
			"Settings:\n"
			"-> fullscreen:      %d\n"
			"-> fps:             %d\n"
			"-> multisamples:    %d\n"
			"-> maxIterations:   %d\n"
			"-> doublePrecision: %d\n"
			"-> nearest:         %d\n"
			"-> numColors:       %d\n",
			fullscreen, fps, multisamples, maxIterations, doublePrecision,
			nearest, numColors
		);
	}
};

// Mandelbrot class
class Mandelbrot{
public:
	// initialize mandelbrot
	int init(int argc, char * argv[]);
	void run();
	void quit();

	void printHelp();
private:
	int parseArguments(int argc, char * argv[]);
	MandelbrotSettings _settings;

	void saveToFile();
	int initWindow();
	void resizeWindowEvent();
	// returns true if application was quit by user
	bool processEvents();
	void updateTransform();
	void updateJuliaCFromMousePos(int, int);
	void render();
	void clearScreen(){glClear(GL_COLOR_BUFFER_BIT);}
	void flipScreen(){SDL_GL_SwapWindow(_mainWindow);}
	void getWorldMousePos(int mouse_x, int mouse_y, double * pos);
	SDL_Window * _mainWindow;
	int _windowW;
	int _windowH;
	bool _redrawEvent;
	GLuint _screenRectBuffer;
	GLuint _sobolBuffer[NUM_SOBOL_MAPS];
	MandelShader _shader;
	GLuint _colorMap;
	double _transform[9];
	double _zoom;
	double _zoomSpeed;
	double _maxZoom;
	double _position[2];
	double _juliaC[2];
	bool _LmousePressed;
	bool _RmousePressed;
	bool _multisampleEnabled;
};

