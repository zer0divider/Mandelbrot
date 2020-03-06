#include "mandelbrot.h"

int main(int argc, char *argv[]){

	Mandelbrot m;
	if(m.init(argc, argv)){
		return 1;
	}
	m.run();
	m.quit();
	return 0;
}

int Mandelbrot::init(int argc, char * argv[])
{

	_juliaC[0] = 0; _juliaC[1] = 0;
	_zoom = MANDELBROT_INITIAL_ZOOM;
	_zoomSpeed = 1.1;
	_position[0] = MANDELBROT_INITIAL_X_OFFSET;
	_position[1] = 0;

	// check for settings file
	const char * file_arg_name =  "args.txt";
	FILE * f = fopen(file_arg_name, "r");
	char *file_argv[128];
	int file_arg_name_len = strlen(file_arg_name);
	file_argv[0] = new char[file_arg_name_len+1];
	strcpy(file_argv[0], file_arg_name);
	if(f){
		printf("Loading settings from 'args.txt'...\n");
		int file_argc = 1;
		char buffer[128];
		while(fscanf(f, "%s", buffer) == 1){
			int buffer_len = strlen(buffer);
			file_argv[file_argc] = new char[buffer_len+1];
			strcpy(file_argv[file_argc], buffer);
			file_argc++;
		}
		if(parseArguments(file_argc, file_argv)){
			return 1;
		}
		fclose(f);
	}

	// parsing arguments from commandline
	if(parseArguments(argc, argv)){
		return 1;
	}

	_settings.print();

	// initialize framework
	if(initWindow()){
		return 1;
	}

	return 0;
}

int Mandelbrot::initWindow()
{
	//initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0){
		printf("Failed to initialize SDL: %s\n", SDL_GetError());
		return 1;
	}

	unsigned int sdl_flags = SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
	//getting current screen resolution if fullscreen activated
	_windowW = 800;
	_windowH = 600;
	if(_settings.fullscreen){
		sdl_flags |= SDL_WINDOW_FULLSCREEN;
		//getting current display-resolution
		SDL_DisplayMode current;
		if (SDL_GetDesktopDisplayMode(0, &current) != 0){
			printf("Warning: Could not retrieve current display resolution: %s\n", SDL_GetError());
		}
		else{
			_windowW = current.w;
			_windowH = current.h;
		}
	}

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
	SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
	if(_settings.doublePrecision){// version 4.0 needed for double precision
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
	}
	else{
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	}
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	_mainWindow = SDL_CreateWindow("Mandelbrot", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, _windowW, _windowH, sdl_flags);
		
	if (_mainWindow == NULL) { //failed to create a window
		printf("Error while creating window: %s\n", SDL_GetError());
		return 1;
	}

	//create gl-context
	SDL_GLContext glContext = SDL_GL_CreateContext(_mainWindow);
	if (glContext == 0){
		printf("Error while creating OpenGL Context: %s\n", SDL_GetError());
		return 1;
	}

	// activate vsync
	SDL_GL_SetSwapInterval(1);

	// init glew
	glewExperimental = GL_TRUE;// support experimental drivers
	GLenum glew_res = glewInit();
	if (glew_res != GLEW_OK){
		printf("Error while initializing GLEW: %s\n", (const char *)glewGetErrorString(glew_res));
		return 1;
	}

	if(_settings.doublePrecision){// if open gl version 4.0 is set create vertex array object
		GLuint vao;
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
	}

	//init GL parameters
	glClearColor(1.f, 1.f, 1.f, 1.f);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	// initializing screen rect buffer
	float rect[] = {-1, -1,
					1, -1,
					-1, 1,
					1, 1};
	glGenBuffers(1, &_screenRectBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, _screenRectBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(rect), rect, GL_STATIC_DRAW);

	glGenBuffers(NUM_SOBOL_MAPS, _sobolBuffer);
	for(int i = 0; i < NUM_SOBOL_MAPS; i++){
		glBindBuffer(GL_ARRAY_BUFFER, _sobolBuffer[i]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float)*(1<<i)*2, SOBOL_MAPS[i], GL_STATIC_DRAW);
	}

	// generate color map (1d texture)
	glGenTextures(1, &_colorMap);
	glBindTexture(GL_TEXTURE_1D, _colorMap);
	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, _settings.numColors, 0, GL_RGBA, GL_UNSIGNED_BYTE, _settings.colors);
	GLint filter = GL_LINEAR;
	if(_settings.nearest)
		filter = GL_NEAREST;
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, filter);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, filter);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

	// compiling shader
	if(_shader.compile(_settings.doublePrecision)){
		return 1;
	}
	_shader.use();
	_shader.setMaxIterations(_settings.maxIterations);
	_shader.setJulia(_settings.julia);
	_multisampleEnabled = false;
	if(_settings.multisamples > 0){
		if(_settings.multisamples > 16){
			puts("Warning: Maximum number of multisamples is 16!");
			_settings.multisamples = 16;
		}
		_shader.setNumSamples(_settings.multisamples);
		_multisampleEnabled = true;
	}
	_LmousePressed = false;
	_RmousePressed = false;
	_shader.setJuliaC(_juliaC);

	//setting viewport
	resizeWindowEvent();
	return 0;
}

void Mandelbrot::resizeWindowEvent(){
	glViewport(0, 0, _windowW, _windowH);
	_shader.setWindowSize(_windowW, _windowH);
	updateTransform();
}

void Mandelbrot::updateTransform(){
	float scale = _zoom;
	/*_transform[0];*/	_transform[3] = 0.0;		_transform[6] = _position[0];
	_transform[1] = 0;	/*_transform[4] = scale;*/ 	_transform[7] = _position[1];
	_transform[2] = 0;	_transform[5] = 0.0; 		_transform[8] = 1.0;
	if(_windowW > _windowH){
		float w_aspect = static_cast<float>(_windowW)/_windowH;
		_transform[0] = scale*w_aspect;
		_transform[4] = scale;
	}
	else{
		float h_aspect = static_cast<float>(_windowH)/_windowW;
		_transform[0] = scale;
		_transform[4] = scale*h_aspect;
	}
	_shader.setTransform(_transform);
}

void Mandelbrot::printHelp()
{
	puts(	"Usage: mandelbrot [options]\n"
			"options:\n"
			"--help                    show this help\n"
			"--fullscreen              sets window to fullscreen mode\n"
			"--framerate <fps>         set framerate\n"
			"--multisamples <samples>  specify number of samples for multisampling (e.g. 2, 4, 8)\n"
			"--max_iterations <value>  number of maximum iterations to determine whether value is in the set\n"
			"--double_precision        use 64 bit floats instead of 32 bit floats (requires GLSL version >= 4.0)\n"
			"--colors <file>           specify a .bmp file containing a colormap\n"
			"--julia                   enables full julia set instead of mandelbrot (start value for z can be selected using the mouse)\n"
			"--nearest                 use nearest texture filtering for the color map instead of linear\n"
			"--location <file>         specify a file from which a location on the fractal is loaded\n"
			"\n"
			"Controls:\n"
			"Move the mouse while pressing down the left mouse button to pan.\n"
			"Use the mouse wheel to zoom in/out.\n"
			"Press <j> to toggle full julia set.\n"
			"When julia set is activated, press the right mouse button to select an offset c in the function f(z) = z^2 + c.\n"
			"Press <r> to reset everything.\n"
			"Press <d>/<h> to double/halfen the current max_iterations.\n"
			"Press <s> to make a screen shot (saved as 'mandelbrot.bmp').\n"
			"Press <m> toggle multisampling (only available if option --multisamples set).\n"
	);
}

void Mandelbrot::run()
{
	_redrawEvent = true;
	while(true){
		Uint32 t_start = SDL_GetTicks();
		if(processEvents()){// rerender only if something changes
			break;
		}
		if(_redrawEvent){
			clearScreen();
			render();
			flipScreen();
			_redrawEvent = false;
		}
		Uint32 t_end = SDL_GetTicks();

		int delay = 1000/_settings.fps - (t_end-t_start);
		if(delay > 0)
			SDL_Delay(delay);
	}
}

void Mandelbrot::quit(){
	SDL_Quit();
}

bool Mandelbrot::processEvents(){
	SDL_Event e;
	while(SDL_PollEvent(&e)){
		switch(e.type)
		{
		case SDL_QUIT:
		{
			return true;
		}break;
		case SDL_WINDOWEVENT:
		{
			if(e.window.event == SDL_WINDOWEVENT_RESIZED)
			{
				_redrawEvent = true;
				_windowW = e.window.data1;
				_windowH = e.window.data2;
				resizeWindowEvent();
			}
		}break;
		case SDL_KEYDOWN:{
			SDL_Keycode keysym = e.key.keysym.sym;
			if(keysym == SDLK_ESCAPE){
				return true;
			}
			else if(keysym == SDLK_j){
				if(e.key.repeat == 0){
					_settings.julia = !_settings.julia;
					_shader.setJulia(_settings.julia);
					_shader.setJuliaC(_juliaC);
					_redrawEvent = true;
				}
			}
			else if(keysym == SDLK_s){
				if(e.key.repeat == 0){
					saveToFile();
				}
			}
			else if(keysym == SDLK_r){
				if(e.key.repeat == 0){
					if(_settings.julia)
						_position[0] = 0;
					else
						_position[0] = MANDELBROT_INITIAL_X_OFFSET;
;
					_position[1] = 0;
					_zoom = MANDELBROT_INITIAL_ZOOM;
					updateTransform();
					_redrawEvent = true;
				}
			}
			else if(keysym == SDLK_d ||
					keysym == SDLK_h){
				if(e.key.repeat == 0){
					if(keysym == SDLK_d){
						_settings.maxIterations *= 2;
					}else{
						_settings.maxIterations /= 2;
						if(_settings.maxIterations < 1){
							_settings.maxIterations = 1;
						}
					}
					_shader.setMaxIterations(_settings.maxIterations);
					_redrawEvent = true;
				}
			}
			else if(keysym == SDLK_m){// toggle multisampling
				if(e.key.repeat == 0 && _settings.multisamples > 0){
					_multisampleEnabled = !_multisampleEnabled;
					_shader.setNumSamples(_multisampleEnabled? _settings.multisamples : 1);
					_redrawEvent = true;
				}
			}
		}break;
		case SDL_MOUSEWHEEL:{
			float zoom_before = _zoom;
			if(e.wheel.y < 0){
				int max = -e.wheel.y;
				for(int i = 0; i < max; i++){
					_zoom *= _zoomSpeed;
				}
			}
			else{
				int max = e.wheel.y;
				for(int i = 0; i < max; i++){
					_zoom /= _zoomSpeed;
				}
			}
			double world_mouse[2];
			double center[2];
			double rel_zoom = zoom_before/_zoom;
			int mouse[2];
			getWorldMousePos(_windowW/2.0f, _windowH/2.0f, center);
			SDL_GetMouseState(mouse, mouse+1);
			getWorldMousePos(mouse[0], mouse[1], world_mouse);
			double delta[2];
			delta[0] = (center[0]-world_mouse[0]);
			delta[1] = (center[1]-world_mouse[1]);
			_position[0] -= delta[0]*rel_zoom -delta[0];
			_position[1] -= delta[1]*rel_zoom -delta[1];
			_redrawEvent = true;
			updateTransform();
		}break;
		case SDL_MOUSEBUTTONDOWN:{
			if(e.button.button == SDL_BUTTON_RIGHT){
				_RmousePressed = true;
				updateJuliaCFromMousePos(e.motion.x, e.motion.y);
				_redrawEvent = true;
			}else if(e.button.button == SDL_BUTTON_LEFT){
				_LmousePressed = true;
			}
		}break;
		case SDL_MOUSEBUTTONUP:{
			if(e.button.button == SDL_BUTTON_RIGHT){
				_RmousePressed = false;
			}else if(e.button.button == SDL_BUTTON_LEFT){
				_LmousePressed = false;
			}
		}break;
		case SDL_MOUSEMOTION:{
			if(_LmousePressed){
				_position[0] -= 2*_transform[0]*e.motion.xrel/static_cast<double>(_windowW);
				_position[1] -= -2*_transform[4]*e.motion.yrel/static_cast<double>(_windowH);
				updateTransform();
				_redrawEvent = true;
			}
			if(_RmousePressed){
				updateJuliaCFromMousePos(e.motion.x, e.motion.y);
				_redrawEvent = true;
			}
		}break;
		default:{
		}
		}
	}
	return false;
}

void Mandelbrot::getWorldMousePos(int mouse_x, int mouse_y, double * pos){
	pos[0] = _transform[0]*(2*mouse_x/static_cast<double>(_windowW) - 1) +  _position[0];
	pos[1] = _transform[4]*(-2*mouse_y/static_cast<double>(_windowH) + 1) + _position[1];
}

void Mandelbrot::updateJuliaCFromMousePos(int mouse_x, int mouse_y)
{
	getWorldMousePos(mouse_x, mouse_y, _juliaC);
	_shader.setJuliaC(_juliaC);
}

int Mandelbrot::parseArguments(int argc, char * argv[])
{
	char * color_path = NULL;
	for(int i = 1; i < argc; i++)
	{
		if(!strcmp(argv[i], "--fullscreen")){
			_settings.fullscreen = true;
		}
		else if(!strcmp(argv[i], "--framerate")){
			i++;
			if(i < argc){
				_settings.fps = atoi(argv[i]);		
				if(_settings.fps < 1){
					_settings.fps = 1;
				}
			}
			else{
				puts("No framerate specified!");
				return 1;
			}
		}
		else if(!strcmp(argv[i], "--max_iterations")){
			i++;
			if(i < argc){
				_settings.maxIterations = atoi(argv[i]);
				if(_settings.maxIterations <= 0){
					_settings.maxIterations = 1;
				}
			}
			else{
				puts("No value specified for --max_iterations!");
				return 1;
			}
		}
		else if(!strcmp(argv[i], "--multisamples")){
			i++;
			if(i < argc){
				_settings.multisamples = atoi(argv[i]);	
				if(_settings.multisamples < 0){
					_settings.multisamples = 0;
				}
			}
			else{
				puts("No value specified for --multisamples!");
				return 1;
			}
		}
		else if(!strcmp(argv[i], "--colors")){
			i++;
			if(i < argc){
				color_path = argv[i];
			}
			else{
				puts("No file specified for --colors!");
				return 1;
			}
		}
		else if(!strcmp(argv[i], "--julia")){
			_settings.julia = true;	
		}
		else if(!strcmp(argv[i], "--double_precision")){
			_settings.doublePrecision = true;	
		}
		else if(!strcmp(argv[i], "--nearest")){
			_settings.nearest = true;	
		}
		else if(!strcmp(argv[i], "--location")){
			i++;
			if(i < argc){
				double d1;
				double d2;
				char buffer[512];
				FILE * f = fopen(argv[i], "r");
				int ret = 0;
				while(fgets(buffer, 512, f))
				{
					int attrib_start = 0;
					int buffer_len = strlen(buffer);
					while(isspace(buffer[attrib_start])){attrib_start++;}
					int attrib_end = attrib_start+1;
					while(!isspace(buffer[attrib_end])){attrib_end++;}
					buffer[attrib_end] = '\0';
					const char * attrib_name = &buffer[attrib_start];
					const char * values = &buffer[attrib_end];
					if(buffer_len > attrib_end+1){
						values = &buffer[attrib_end+1];
					}
					if(!strcmp(attrib_name, "position")){
						if(sscanf(values, "%lf %lf", &_position[0], &_position[1]) == 2){
							printf("Setting position to (%.20f, %.20f)\n", _position[0], _position[1]);
						}else{
							printf("Error: Expected 2 values for attribute '%s'!\n", attrib_name);
							break;
						}
					}
					else if(!strcmp(attrib_name, "zoom")){
						if(sscanf(values, "%lf", &_zoom) == 1){
							printf("Setting zoom to %.20f\n", _zoom);
						}else{
							printf("Error: Expected 1 values for attribute '%s'!\n", attrib_name);
							break;
						}
					}
					else if(!strcmp(attrib_name, "julia_c")){
						if(sscanf(values, "%lf %lf", &_juliaC[0], &_juliaC[1]) == 2){
							printf("Setting full julia set c offset to (%.20f, %.20f)\n", _juliaC[0], _juliaC[1]);
							_settings.julia = true;
						}else{
							printf("Error: Expected 2 values for attribute '%s'!\n", attrib_name);
							break;
						}
					}
					else if(!strcmp(attrib_name, "iterations")){
						if(sscanf(values, "%d", &_settings.maxIterations) == 1){
							printf("Setting max. iterations to %d\n", _settings.maxIterations);
						}else{
							printf("Error: Expected 2 values for attribute '%s'!\n", attrib_name);
							break;
						}
					}
					else{
						printf("Warning: Unknown attribute '%s' encountered while loading location from '%s'!\n", attrib_name, argv[i]);
					}
				}
				fclose(f);
			}
			else{
				puts("No file specfied for --location");
				return 1;
			}
		}
		else if(!strcmp(argv[i], "--help") ||
				!strcmp(argv[i], "-h")){
			printHelp();
			return 1;	
		}
		else{
			printf("Unknown option '%s'!\n", argv[i]);
			return 1;
		}
	}

	if(color_path != NULL){
		//open file for reading
		SDL_Surface * s = SDL_LoadBMP(color_path);
		if(s == NULL){
			puts(SDL_GetError());
			return 1;
		}
		int min = MANDELBROT_MAX_COLORS;
		if(s->w < min)
			min = s->w;
		_settings.numColors = min;
		switch(s->format->BytesPerPixel){
			case 1:{
			for(int i = 0; i < min ; i++){
				Uint8 value = ((Uint8*)s->pixels)[i];
				_settings.colors[i] = 0xFF000000 | value<<16 | value<<8 | value;
			}
			}break;
			case 2:{
			for(int i = 0; i < min ; i++){
				_settings.colors[i] = 0xFF000000 | ((Uint16*)s->pixels)[i];
			}
			}break;
			case 3:
			case 4:{
				int R_bit_pos = 0, G_bit_pos = 0, B_bit_pos = 0, A_bit_pos;
			if(s->format->Rmask == 0x000000FF){
				R_bit_pos = 0;
				A_bit_pos = 24;
				if(s->format->Gmask == 0x0000FF00){
					G_bit_pos = 8;
					B_bit_pos = 16;
				}
				else{
					printf("Unrecognized color format in '%s'!\n", color_path);
					SDL_FreeSurface(s);
					return 1;
				}
			}
			else if(s->format->Rmask == 0xFF000000){
				R_bit_pos = 24;
				A_bit_pos = 0;
				if(s->format->Gmask == 0x00FF0000){
					G_bit_pos = 16;
					B_bit_pos = 8;
				}
				else{
					printf("Unrecognized color format in '%s'!\n", color_path);
					SDL_FreeSurface(s);
					return 1;
				}
			}
			else if(s->format->Rmask == 0x00FF0000){
				R_bit_pos = 16;
				A_bit_pos = 24;
				if(s->format->Gmask == 0x0000FF00){
					G_bit_pos = 8;
					B_bit_pos = 0;
				}
				else{
					printf("Unrecognized color format in '%s'!\n", color_path);
					SDL_FreeSurface(s);
					return 1;
				}
			}
			else{
				printf("Unrecognized color format in '%s'!\n", color_path);
				SDL_FreeSurface(s);
				return 1;
			}
			if(s->format->BytesPerPixel == 3){
				for(int i = 0; i < min*3; i++){
					Uint32 r = ((Uint8*)s->pixels)[i*3 + 2];
					Uint32 g = ((Uint8*)s->pixels)[i*3 + 1]<<8;
					Uint32 b = ((Uint8*)s->pixels)[i*3 + 0]<<16;
					_settings.colors[i] = 	r | g | b | 0xFF000000;	
				}
			}
			else{
				for(int i = 0; i < min ; i++){
					Uint32 color = ((Uint32*)s->pixels)[i];
					_settings.colors[i] = 	((color&s->format->Rmask)>>R_bit_pos) |
											(((color&s->format->Gmask)>>G_bit_pos)<<8) |
											(((color&s->format->Bmask)>>B_bit_pos)<<16) |
											(((color&s->format->Amask)>>A_bit_pos)<<24);
				}
			}
			}break;
		}
		SDL_FreeSurface(s);
	}

	return 0;
}

void Mandelbrot::render(){
	GLint vertex_loc = _shader.getVertexLocation();
	glEnableVertexAttribArray(vertex_loc);

	// drawing rectangle covering the whole screen
	glBindBuffer(GL_ARRAY_BUFFER, _screenRectBuffer);
	glVertexAttribPointer(vertex_loc, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	/*
	// visualizing sobol patterns
	glPointSize(10);
	int sobol_index = 3;
	glBindBuffer(GL_ARRAY_BUFFER, _sobolBuffer[sobol_index]);
	glVertexAttribPointer(vertex_loc, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glDrawArrays(GL_POINTS, 0, 1<<sobol_index);
	*/
}

void Mandelbrot::saveToFile(){
	const char * path = "mandelbrot.bmp";
	SDL_Surface *s = SDL_CreateRGBSurface(0, _windowW, _windowH, 32, 0x000000FF,0x0000FF00,0x00FF0000,0xFF000000);
	glReadPixels(0, 0, _windowW, _windowH, GL_RGBA, GL_UNSIGNED_BYTE, s->pixels);
	int size = _windowW*_windowH;
	Uint32 * pixels = ((Uint32*)s->pixels);
	for(int i =0; i < size/2; i++){
		int x = i%_windowW;
		int y = i/_windowW;
		int mirrored_index = (_windowH-y-1)*_windowW + x;
		Uint32 p = pixels[mirrored_index];
		pixels[mirrored_index] = pixels[i];
		pixels[i] = p;
	}
	if(SDL_SaveBMP(s, path)){
		puts(SDL_GetError());
	}
	SDL_FreeSurface(s);

	// saving text file with parameters
	const char * location_path = "mandelbrot.bmp.txt";
	FILE * f = fopen(location_path, "w");
	if(f == NULL){
		printf("Failed to save location file '%s'!", location_path);
	}
	else{
		fprintf(f, "position %.20f %.20f\nzoom %.20f\niterations %d", _position[0], _position[1], _zoom, _settings.maxIterations);
		if(_settings.julia){
			fprintf(f, "\njulia_c %.20f %.20f", _juliaC[0], _juliaC[1]);
		}
		fclose(f);
	}
}
