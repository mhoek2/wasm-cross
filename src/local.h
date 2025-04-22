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
// emcc src/main.cpp src/shader.cpp src/frame.cpp src/camera.cpp src/framebuffer.cpp -o index.html -s FULL_ES3=1 -s USE_GLFW=3 --preload-file src/glsl -I./external-wasm
//
#include <emscripten.h>
#include <GLES3/gl3.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>  // for glm::value_ptr

#define SCREEN_W 1200
#define SCREEN_H 800

struct vec2_t {
	float x;
	float y;
};

struct vec3_t {
	float x;
	float y;
	float z;
};

struct mat4_t {
	float m[16];
};

typedef struct {
	glm::vec3	camera_pos;
	glm::vec3	camera_front;
	glm::vec3	camera_up;
	glm::vec3	camera_right;
	float		mouse_sensitivity;
	float		move_velocity;
	float		jaw;
	float		pitch;
	float		last_x;
	float		last_y;
	bool		ignore_next_mouse_event;
	bool		mouse_moving;
} camera_t;

typedef struct {
	glm::mat4	projection;
	glm::mat4	view;
	float		aspect_ratio;
} renderer_t;

struct framebuffer_t {
	GLuint		fbo;
	GLuint		attachment;
};

extern GLFWwindow* g_window;

extern GLuint	screenVAO, screenVBO;
extern GLuint	color_shader;
extern GLuint	gamma_shader;

extern framebuffer_t	fbo;
extern framebuffer_t*	current_fbo;

extern camera_t		camera;
extern renderer_t	renderer;

const vec2_t screen_size = { SCREEN_W , SCREEN_H };
const vec2_t screen_center = { SCREEN_W / 2 , SCREEN_H / 2 };

// shader
void	use_shader(GLuint shader);
int		init_shaders(void);

// frame
void	begin_frame(void);
void	end_frame(void);

// camera
int		init_camera(void);
glm::mat4 get_view_matrix(void);
void	process_keyboard(GLFWwindow* window);
void	mouse_callback(GLFWwindow* window, double xpos, double ypos);

// framebuffer
void	unbind_fbo(void);
void	bind_fbo(framebuffer_t* fbo);
void	render_fbo(framebuffer_t* fbo);
void	create_screen_vao();
int		init_framebuffer(void);
framebuffer_t create_fbo_with_depth(const vec2_t& size);


#endif // LOCAL_H