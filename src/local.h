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
// emcc src/main.cpp src/shader.cpp src/frame.cpp src/camera.cpp src/framebuffer.cpp src/debug.cpp -o build-wasm/index.html -s FULL_ES3=1 -s USE_GLFW=3 --preload-file src/glsl -I./external-wasm -v
//
#include <emscripten.h>
#include <GLES3/gl3.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#ifndef M_PI
	#define M_PI 3.14159265358979323846
#endif

#define SCREEN_W 1200
#define SCREEN_H 800

#define SHADOW_RES 4096

struct vec2_t {
	float x;
	float y;
};

struct vec3_t {
	float x;
	float y;
	float z;
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
	glm::mat4	identity;
	float		aspect_ratio;
} renderer_t;

struct framebuffer_t {
	GLuint		fbo;
	GLuint		attachment;
};

typedef struct {
	GLFWwindow	*g_window;
	camera_t	camera;
	renderer_t	renderer;
	float		time;

	struct {
		framebuffer_t	main;
		framebuffer_t	shadow;
		framebuffer_t	*current;
	} framebuffers;

	struct {
		GLuint vao;
		GLuint vbo;
	} screen;

	struct {
		GLuint color_shader;
		GLuint gamma_shader;
		GLuint earth_shader;
		GLuint shadow_depth;
		GLuint shadow_debug;
		GLuint atmosphere_shader;
	} shaders;

	struct {
		GLuint vao;
		GLuint vbo[3];
		GLuint ibo;
		GLuint colormap_tex;
		GLuint nightmap_tex;
		GLuint heightmap_tex;
		GLuint aomap_tex;
		GLuint normalmap_tex;
		GLuint metallicmap_tex;
		GLuint roughnessmap_tex;
		GLuint specularmap_tex;
	} earth;

	struct {
		GLuint vao;
		GLuint vbo[3];
		GLuint ibo;
		GLuint noise_tex;
	} atmosphere;

} application_t;

extern application_t app;

const vec2_t screen_size = { SCREEN_W , SCREEN_H };
const vec2_t screen_center = { SCREEN_W / 2 , SCREEN_H / 2 };


// shader
void	use_shader(GLuint shader);
int		init_shaders( void );

// frame
void	prepare_frame( void );
void	begin_frame( void );
void	end_frame( void );

// camera
int		init_camera( void );
glm::mat4 get_view_matrix( void );
void	process_keyboard( GLFWwindow* window );
void	mouse_callback( GLFWwindow* window, double xpos, double ypos );

// framebuffer
void	unbind_fbo( void );
void	bind_fbo( framebuffer_t* fbo );
void	render_fbo( framebuffer_t* fbo );
void	create_screen_vao( void );
int		init_framebuffer( void );

// debug
void	setup_triangle( void );
void	draw_triangle( void );
void	setup_cube( void );
void	draw_cube( void );
void	setup_shadow_map_debug( void );
void	draw_shadow_map_debug( void );

// earth
void	prepare_uniforms( void );
void	setup_earth( void );
void	draw_earth( GLuint shader );
void	draw_atmosphere( GLuint shader );
void	render_shadow_pass( GLuint shader );

#endif // LOCAL_H