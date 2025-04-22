#ifndef LOCAL_H
#define LOCAL_H

#ifndef __EMSCRIPTEN__
#define GLEW_STATIC
#include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>

#ifdef __EMSCRIPTEN__
// compile using
// .\emsdk_env.bat  
// emcc src/main.cpp -o index.html -s FULL_ES3=1 -s USE_GLFW=3
// emcc src/main.cpp src/shader.cpp -o index.html -s FULL_ES3=1 -s USE_GLFW=3 --preload-file src/glsl
// emcc src/main.cpp src/shader.cpp -o index.html -s FULL_ES3=1 -s USE_GLFW=3 --preload-file src/glsl -I./external-wasm
// emcc src/main.cpp src/shader.cpp src/framebuffer.cpp -o index.html -s FULL_ES3=1 -s USE_GLFW=3 --preload-file src/glsl -I./external-wasm
// emcc src/main.cpp src/shader.cpp src/frame.cpp src/framebuffer.cpp -o index.html -s FULL_ES3=1 -s USE_GLFW=3 --preload-file src/glsl -I./external-wasm
//
#include <emscripten.h>
#include <GLES3/gl3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#endif

#define SCREEN_W 1200
#define SCREEN_H 800

struct vec2_t {
	float x;
	float y;
};

struct mat4_t {
	float m[16];
};

struct framebuffer_t {
	GLuint fbo;
	GLuint attachment;
};

extern GLFWwindow* g_window;

extern GLuint screenVAO, screenVBO;
extern GLuint color_shader;
extern GLuint gamma_shader;

extern framebuffer_t fbo;
extern framebuffer_t* current_fbo;

const vec2_t screen_size = { SCREEN_W , SCREEN_H };

// shader
void	use_shader(GLuint shader);
int		init_shaders(void);

// frame
void	begin_frame(void);
void	end_frame(void);

// framebuffer
void	unbind_fbo(void);
void	bind_fbo(framebuffer_t* fbo);
void	render_fbo(framebuffer_t* fbo);
void	create_screen_vao();
int		init_framebuffer(void);
framebuffer_t create_fbo_with_depth(const vec2_t& size);


#endif // LOCAL_H