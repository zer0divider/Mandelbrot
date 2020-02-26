#include "mandel_shader.h"

int MandelShader::compile(bool d)
{
	_doublePrecision = d;
	GLint success = 0;
	int error = 0;
	//create and compile vertex shader
	GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, &MANDEL_VERTEX_SHADER, 0);
	glCompileShader(vertex_shader);
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
	if(success == GL_FALSE){
		puts("Error during vertex shader compilation!");
		error++;
	}

	//create and compile fragment shader
	GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, d ? &MANDEL_FRAGMENT_SHADER_DOUBLE : &MANDEL_FRAGMENT_SHADER, 0);
	glCompileShader(fragment_shader);
	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
	if(success == GL_FALSE){
		puts("Error during fragment shader compilation!");
		error++;	
	}

	//create and link program with vs and fs
	_programID = glCreateProgram();
	glAttachShader(_programID, vertex_shader);
	glAttachShader(_programID, fragment_shader);
	glLinkProgram(_programID);
	glGetShaderiv(_programID, GL_LINK_STATUS, &success);
	if(success == GL_FALSE){
		puts("Error during program linking!");
		error++;	
	}

	//Error-handling:
		static const int bufSize = 1024;
		GLchar *buffer = new GLchar [bufSize];
		//Vertex Shader Error-Log
		glGetShaderInfoLog(vertex_shader, bufSize, 0, buffer);
		buffer[bufSize-1] = '\0';
		if(buffer[0] != '\0')//non-empty
		{
			puts((const char*) buffer);
		}
		//Fragment Shader Error-Log
		glGetShaderInfoLog(fragment_shader, bufSize, 0, buffer);
		buffer[bufSize-1] = '\0';
		if(buffer[0] != '\0')//non-empty
		{
			puts((const char*) buffer);
		}

		//Program Error-Log
		glGetProgramInfoLog(_programID, bufSize, 0, buffer);
		buffer[bufSize-1] = '\0';
		if(buffer[0] != '\0')//non-empty
		{
			puts((const char*) buffer);
		}
		delete[] buffer;
	/////
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	// getting locations
	if(_programID > 0){
		_vertexLocation = glGetAttribLocation(_programID, "vertex");
		_windowSizeLocation = glGetUniformLocation(_programID, "window_size");
		_colorMapLocation = glGetUniformLocation(_programID, "color_map");
		_maxIterationsLocation = glGetUniformLocation(_programID, "max_iterations");
		_transformLocation = glGetUniformLocation(_programID, "transform");
		_juliaCLocation = glGetUniformLocation(_programID, "julia_c");
		_juliaLocation = glGetUniformLocation(_programID, "julia");
		glUniform1i(_colorMapLocation, 0);// default target: 0
	}
	//return number of errors
	return error;
}
