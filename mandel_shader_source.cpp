const char * MANDEL_VERTEX_SHADER = 
	"#version 120\n"
	"attribute vec2 vertex;\n"
	"void main(void){\n"
	"  gl_Position = vec4(vertex, 0, 1);\n"
	"\n"
	"}"
;

#define SOBOL_MAP_DECLARATION \
"uniform int num_samples = 1;\n" \
"uniform vec2 sobol_map[16];\n"

#define SOBOL_SAMPLING_START \
"for(int sample_i = 0; sample_i < num_samples; sample_i++){\n"

#define SOBOL_SAMPLING_END \
" }\n" \
"color /= float(num_samples);\n" \

const char * MANDEL_FRAGMENT_SHADER = 
	"#version 120\n"
	"uniform int max_iterations;\n"
	"uniform mat3 transform;\n"
	"uniform vec2 window_size;\n"
	"uniform vec2 julia_c;\n"
	"uniform int julia = 0;\n"
	"uniform int ms = 0;\n"
	SOBOL_MAP_DECLARATION
	"uniform sampler1D color_map;\n"
	"vec2 mandel_iterate(vec2 z, vec2 c){\n"
	"  return vec2(z.x*z.x - z.y*z.y + c.x, 2*z.x*z.y + c.y);\n"
	"}\n"
	"float lensqrd(vec2 v){return v.x*v.x + v.y*v.y;}\n"
	"void main(void){\n"
	"  vec4 color = vec4(0);\n"
	SOBOL_SAMPLING_START
	"  vec2 p = vec2(2*(gl_FragCoord.xy+sobol_map[sample_i])/window_size - vec2(1, 1));\n"
	"  p = (transform*vec3(p, 1)).xy;\n"
	"  float s = 1;"
	"  if(julia == 0){\n"
	"  vec2 z = vec2(0,0);\n"
	"  for(int i = 0; i < max_iterations; i++){\n"
	"    if(lensqrd(z) > 4.0){s = float(i)/float(max_iterations-1); break;}\n"
	"    z = mandel_iterate(z, p);\n"
	"  }\n"
	"  }else{\n"
	"  vec2 z = p;\n"
	"  for(int i = 0; i < max_iterations; i++){\n"
	"    if(lensqrd(z) >= 4.0){s = float(i)/float(max_iterations-1); break;}\n"
	"    z = mandel_iterate(z, julia_c);\n"
	"  }\n"
	"  }\n"
	"  color += texture1D(color_map, s);\n"
	SOBOL_SAMPLING_END
	"  gl_FragColor = color;"
	"}"
;

// double precision
const char * MANDEL_FRAGMENT_SHADER_DOUBLE = 
	"#version 400\n"
	"out vec4 color;\n"
	"uniform int max_iterations;\n"
	"uniform dmat3 transform;\n"
	"uniform vec2 window_size;\n"
	"uniform dvec2 julia_c;\n"
	"uniform int julia = 0;\n"
	SOBOL_MAP_DECLARATION
	"uniform sampler1D color_map;\n"
	"dvec2 mandel_iterate(dvec2 z, dvec2 c){\n"
	"  return dvec2(z.x*z.x - z.y*z.y + c.x, 2*z.x*z.y + c.y);\n"
	"}\n"
	"double lensqrd(dvec2 v){return sqrt(v.x*v.x + v.y*v.y);}\n"
	"void main(void){\n"
	"  color = vec4(0, 0, 0, 0);\n"
	SOBOL_SAMPLING_START
	"  dvec2 p = dvec2(2*(gl_FragCoord.xy+sobol_map[sample_i])/window_size - dvec2(1, 1));\n"
	"  p = (transform*dvec3(p, 1)).xy;\n"
	"  float s = 1;\n"
	"  if(julia == 0){\n"
	"  dvec2 z = dvec2(0,0);\n"
	"  for(int i = 0; i < max_iterations; i++){\n"
	"    if(lensqrd(z) >= 4.0){s = float(i)/float(max_iterations-1); break;}\n"
	"    z = mandel_iterate(z, p);\n"
	"  }\n"
	"  }else{\n"
	"  dvec2 z = p;\n"
	"  for(int i = 0; i < max_iterations; i++){\n"
	"    if(lensqrd(z) >= 4.0){s = float(i)/float(max_iterations-1); break;}\n"
	"    z = mandel_iterate(z, julia_c);\n"
	"  }\n"
	"  }\n"
	"  color += texture(color_map, s);\n"
	SOBOL_SAMPLING_END
	"}"
;

const float SOBOL_MAP_1[2] = {
	0, 0
};

const float SOBOL_MAP_2[4] = {
	0.25, 0.25,
	-0.25, -0.25,

};

const float SOBOL_MAP_4[8] = {
	 0.25, 	 0.0,
	-0.25, 	 0.0,
	 0.0,	 0.25,
	 0.0,	-0.25,
};

#define THIRD 0.33333
const float SOBOL_MAP_8[16] = {
	 0.25, 	 	 0.0,
	-0.25,		 0.0,
	-THIRD,		 THIRD,
		0,		 THIRD,
	 THIRD,		 THIRD,
	-THIRD,		-THIRD,
		0,		-THIRD,
	 THIRD,		-THIRD,
};

const float SOBOL_MAP_16[32] = {
	-0.375, 0.375,
	-0.125, 0.375,
	 0.125, 0.375,
	 0.375, 0.375,

	-0.375, 0.125,
	-0.125, 0.125,
	 0.125, 0.125,
	 0.375, 0.125,

	-0.375, -0.125,
	-0.125, -0.125,
	 0.125, -0.125,
	 0.375, -0.125,
	
	-0.375, -0.375,
	-0.125, -0.375,
	 0.125, -0.375,
	 0.375, -0.375
};

const float *SOBOL_MAPS[5] = {
	SOBOL_MAP_1,
	SOBOL_MAP_2,
	SOBOL_MAP_4,
	SOBOL_MAP_8,
	SOBOL_MAP_16
};
