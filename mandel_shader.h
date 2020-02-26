#include "glew/glew.h"
#include <stdio.h>

extern const char * MANDEL_VERTEX_SHADER;
extern const char * MANDEL_FRAGMENT_SHADER;
extern const char * MANDEL_FRAGMENT_SHADER_DOUBLE;

class MandelShader{
public:
	// compile shader
	int compile(bool double_precision);
	void use(){glUseProgram(_programID);}
	GLint getVertexLocation(){return _vertexLocation;}
	void setWindowSize(int w, int h){glUniform2f(_windowSizeLocation, w, h);}
	void setTransform(double * mat3){
		if(!_doublePrecision){
			float mat3f[9];
			for(int i = 0; i < 9; i++)
				mat3f[i] = static_cast<float>(mat3[i]);
			glUniformMatrix3fv(_transformLocation, 1, GL_FALSE, mat3f);
		}
		else{
			glUniformMatrix3dv(_transformLocation, 1, GL_FALSE, mat3);
		}
	}
	void setMaxIterations(int max_i){glUniform1i(_maxIterationsLocation, max_i);}
	void setJuliaC(double *c){
		if(!_doublePrecision)
			glUniform2f(_juliaCLocation, c[0], c[1]);
		else
			glUniform2d(_juliaCLocation, c[0], c[1]);
	}
	void setJulia(bool enabled){glUniform1i(_juliaLocation, enabled ? 1 : 0);}
private:
	bool _doublePrecision;
	GLuint _programID;
	GLint _vertexLocation;
	GLint _windowSizeLocation;
	GLint _transformLocation;
	GLint _colorMapLocation;
	GLint _maxIterationsLocation;
	GLint _juliaCLocation;
	GLint _juliaLocation;
};
